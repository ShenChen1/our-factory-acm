#!/usr/bin/env python
# coding: utf-8
'''
script for hanlding pronto infomation
'''
import csv
import json
import re
import sys
from collections import OrderedDict


class ProntoAnalyzer(object):
    '''
    class for Pronto Info Analysis
    '''
    def __init__(self):
        self.keys_in_json = [
            "PR ID", "Group In Charge", "Reported Date",
            "Author", "Author Group", "State", "Transfer Path",
            "Transfer times", "Solving Path", "Pingpong times",
            "Attached PRs", "Attached", "Attached To"]
        self.pronto_dict_init()

    def pronto_dict_init(self):
        '''
        init dict for store pronto info
        '''
        self.prdict = OrderedDict().fromkeys(self.keys_in_json, "")

    def direct_copy_data(self, input_dict):
        '''
        direct copied data from input
        '''
        self.prdict["PR ID"] = input_dict["Problem ID"]
        self.prdict["Group In Charge"] = input_dict["Group in Charge"]
        self.prdict["Reported Date"] = input_dict["Reported Date"]
        self.prdict["Author"] = input_dict["Author"]
        self.prdict["Author Group"] = input_dict["Author Group"]
        self.prdict["State"] = input_dict["State"]
        self.prdict["Attached PRs"] = input_dict["Attached PRs"]

    @classmethod
    def is_the_same_group(cls, group_a, group_b):
        '''
        Note: only ece group is checked here
        '''
        ece_group = {
            "ECE_DEV_FOU", "ECE_DEV_FOU_OAM_MZ",
            "ECE_DEV_FOU_OAM_MZ_EXT", "MANO_HZH_MZOM",
            "MANO_HZH_MZOM_EXT", "ECE_DEV_FOU_UP_L2_HI",
            "ECE_DEV_FOU_UP_L2_LO"}
        if (group_a == group_b) or (
                (group_a in ece_group) and (group_b in ece_group)):
            return True
        return False

    @classmethod
    def handle_tranfer_groups(cls, group, group_in_charge):
        '''
        input: the transfer groups grep in reversion history
        output: the tranfer path
        '''
        path = []
        g_from_last = ""
        g_to_last = ""
        current_group = ""
        for g_from, g_to in group[::-1]:
            g_from = g_from.strip()
            g_to = g_to.strip()
            if ((g_from, g_to) == (g_from_last, g_to_last) or
                    (ProntoAnalyzer.is_the_same_group(
                        g_from, g_to))):
                continue
            else:
                g_from_last = g_from
                g_to_last = g_to
            if not ProntoAnalyzer.is_the_same_group(
                    current_group, g_from):
                path.append(g_from)
                current_group = path[-1]
            if not ProntoAnalyzer.is_the_same_group(
                    current_group, g_to):
                path.append(g_to)
                current_group = path[-1]
        if not ProntoAnalyzer.is_the_same_group(
                current_group, group_in_charge):
            path.append(group_in_charge)
        return path

    @classmethod
    def get_transfer_path(cls, input_dict):
        """
        format looks like:
        The group in charge changed from ECE_DEV_FOU to RCPSEC
        """
        history = input_dict["Revision History"]
        group = []
        path = []
        group_in_charge = input_dict["Group in Charge"].strip()
        pattern = re.compile(
            "The group in charge changed from "
            "([\\w|\\s|\\-|&]*) [T|t]o "
            "([\\w|\\s|\\-|&]*)")
        for line in history.split('\n'):
            group += pattern.findall(line)
        if group:
            path = ProntoAnalyzer.handle_tranfer_groups(group, group_in_charge)
        return path

    @classmethod
    def get_solving_path(cls, transfer_path):
        '''
        get actual problem solving path excluding ping-pong
        '''
        solving_path = []
        pingpong = 0
        pos = 0
        position = dict.fromkeys(transfer_path, -1)
        for transfer_group in transfer_path:
            if position[transfer_group] == -1:
                solving_path.append(transfer_group)
                position[transfer_group] = pos
                pos += 1
            else:
                pingpong += 1
                new_path = []
                for solving_group in solving_path:
                    if position[solving_group] < position[transfer_group]:
                        new_path.append(solving_group)
                    else:
                        break
                new_path.append(transfer_group)
                solving_path = new_path
        return solving_path, pingpong

    def transfer_analysis(self, input_dict):
        '''
        transfering analysis:
        transfer path, solving path, pingpong times
        '''
        path = ProntoAnalyzer.get_transfer_path(input_dict)
        if path:
            self.prdict["Transfer Path"] = path
            self.prdict["Transfer times"] = len(path)
            solving_path, pingpong = ProntoAnalyzer.get_solving_path(path)
            self.prdict["Solving Path"] = solving_path
            self.prdict["Pingpong times"] = pingpong
        else:
            path = []
            path.append(input_dict["Group in Charge"].strip())
            self.prdict["Transfer Path"] = path
            self.prdict["Transfer times"] = 1
            self.prdict["Solving Path"] = path
            self.prdict["Pingpong times"] = 0

    def attached_info(self, input_dict, attached_dict):
        '''
        attached info
        '''
        attached_info_list = input_dict["Attached PRs"].split(", ")
        intersection = list(set(attached_info_list).intersection(
            set(attached_dict)))
        if input_dict["Attached PRs"] == "< empty >":
            self.prdict["Attached"] = "no"
            self.prdict["Attached To"] = ""
        elif not intersection:
            attached_dict[input_dict["Problem ID"]] = ""
            self.prdict["Attached"] = "no"
            self.prdict["Attached To"] = ""
        else:
            self.prdict["Attached"] = "yes"
            if attached_dict[intersection[0]]:
                self.prdict["Attached To"] = attached_dict[intersection[0]]
            else:
                self.prdict["Attached To"] = intersection[0]
                attach_id = input_dict["Problem ID"]
                attached_dict[attach_id] = self.prdict["Attached To"]

    def pronto_info_analysis(self, input_dict, attached_dict):
        '''
        pronto info analysis
        '''
        self.pronto_dict_init()
        self.direct_copy_data(input_dict)
        self.transfer_analysis(input_dict)
        self.attached_info(input_dict, attached_dict)
        return self.prdict

    @classmethod
    def is_header(cls, line):
        '''
        is_header
        '''
        return set(['Problem ID', 'Title', 'Description']) < set(line)

    @classmethod
    def get_data(cls, filename):
        '''
        get_data
        '''
        with open(filename, 'rt', encoding='ascii', errors='ignore') as csv_f:
            csv_reader = csv.reader(csv_f)
            for row in csv_reader:
                yield row

    @classmethod
    def output_with_json(cls, pronto_info_list):
        '''
        output_with_json
        '''
        json_dict = {}
        json_dict["records"] = pronto_info_list
        json_dict["total"] = len(json_dict["records"])
        json_data = json.dumps(json_dict, indent=4)
        print(json_data)

    def get_result(self, file_name):
        '''
        read_csv
        '''
        info_list = []
        csvdict = {}
        attached_dict = {}
        csv_file = ProntoAnalyzer.get_data(file_name)
        csvheader = next(csv_file)
        while not ProntoAnalyzer.is_header(csvheader):
            csvheader = next(csv_file)
        for line in csv_file:
            csvdict = OrderedDict(zip(csvheader, line))
            new_info = self.pronto_info_analysis(csvdict, attached_dict)
            info_list.append(new_info)
        return info_list


def main():
    '''
    main
    '''
    if len(sys.argv) < 2:
        print('need input file..')
        exit(-1)
    analyzer = ProntoAnalyzer()
    pronto_info_list = analyzer.get_result(sys.argv[1])
    analyzer.output_with_json(pronto_info_list)


if __name__ == '__main__':
    main()
