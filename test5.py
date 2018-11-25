# -*- coding: utf-8 -*-
# Package imports
import numpy as np
import csv
import os
import sys

class LogisticRegression:
    def __init__(self, lr=0.01, num_iter=100000, fit_intercept=True, verbose=False):
        self.lr = lr
        self.num_iter = num_iter
        self.fit_intercept = fit_intercept
        self.verbose = verbose

    def __add_intercept(self, x):
        intercept = np.ones((x.shape[0], 1))
        return np.concatenate((intercept, x), axis=1)

    def __sigmoid(self, z):
        return 1 / (1 + np.exp(-z))

    def __loss(self, h, y):
        return (-y * np.log(h) - (1 - y) * np.log(1 - h)).mean()

    def fit(self, x, y):
        if self.fit_intercept:
            x = self.__add_intercept(x)

        # weights initialization
        self.theta = np.zeros(x.shape[1])

        for i in range(self.num_iter):
            z = np.dot(x, self.theta)
            h = self.__sigmoid(z)
            gradient = np.dot(x.T, (h - y)) / y.size
            self.theta -= self.lr * gradient

            z = np.dot(x, self.theta)
            h = self.__sigmoid(z)
            loss = self.__loss(h, y)

            if (self.verbose == True and i % 10000 == 0):
                print('loss: {loss} \t')

    def predict_prob(self, x):
        if self.fit_intercept:
            x = self.__add_intercept(x)
        return self.__sigmoid(np.dot(x, self.theta))

    def predict(self, x):
        return self.predict_prob(x).round()


num_examples = 0    # training set size
nn_input_dim = 2 # input layer dimensionality 
nn_output_dim = 2   # output layer dimensionality

# Gradient descent parameters (I picked these by hand)
epsilon = 0.01  # learning rate for gradient descent
reg_lambda = 0.01  # regularization strength

# Helper function to evaluate the total loss on the dataset
def calculate_loss(model, x):
    w1, b1, w2, b2 = model['w1'], model['b1'], model['w2'], model['b2']
    # Forward propagation to calculate our predictions
    z1 = x.dot(W1) + b1
    a1 = np.tanh(z1)
    z2 = a1.dot(W2) + b2
    exp_scores = np.exp(z2)
    probs = exp_scores / np.sum(exp_scores, axis=1, keepdims=True)
    # Calculating the loss
    corect_logprobs = -np.log(probs[range(num_examples), y])
    data_loss = np.sum(corect_logprobs)
    # Add regulatization term to loss (optional)
    data_loss += reg_lambda/2 * (np.sum(np.square(w1)) + np.sum(np.square(w2)))
    return 1./num_examples * data_loss

# Helper function to predict an output (0 or 1)
def predict(model, x):
    w1, b1, w2, b2 = model['w1'], model['b1'], model['w2'], model['b2']
    # Forward propagation
    z1 = x.dot(w1) + b1
    a1 = np.tanh(z1)
    z2 = a1.dot(w2) + b2
    exp_scores = np.exp(z2)
    probs = exp_scores / np.sum(exp_scores, axis=1, keepdims=True)
    return np.argmax(probs, axis=1)

# This function learns parameters for the neural network and returns the model.
# - nn_hdim: Number of nodes in the hidden layer
# - num_passes: Number of passes through the training data for gradient descent
# - print_loss: If True, print the loss every 1000 iterations
def build_model(nn_hdim, x, y, num_passes=20000, print_loss=False):
    # Initialize the parameters to random values. We need to learn these.
    np.random.seed(0)
    w1 = np.random.randn(nn_input_dim, nn_hdim) / np.sqrt(nn_input_dim)
    b1 = np.zeros((1, nn_hdim))
    w2 = np.random.randn(nn_hdim, nn_output_dim) / np.sqrt(nn_hdim)
    b2 = np.zeros((1, nn_output_dim))

    # This is what we return at the end
    model = {}

    # Gradient descent. For each batch...
    for i in range(0, num_passes):

        # Forward propagation
        z1 = x.dot(w1) + b1
        a1 = np.tanh(z1)
        z2 = a1.dot(w2) + b2
        exp_scores = np.exp(z2)
        probs = exp_scores / np.sum(exp_scores, axis=1, keepdims=True)

        # Backpropagation
        delta3 = probs
        delta3[range(num_examples), y] -= 1
        dw2 = (a1.T).dot(delta3)
        db2 = np.sum(delta3, axis=0, keepdims=True)
        delta2 = delta3.dot(w2.T) * (1 - np.power(a1, 2))
        dw1 = np.dot(x.T, delta2)
        db1 = np.sum(delta2, axis=0)

        # Add regularization terms (b1 and b2 don't have regularization terms)
        dw2 += reg_lambda * w2
        dw1 += reg_lambda * w1

        # Gradient descent parameter update
        w1 += -epsilon * dw1
        b1 += -epsilon * db1
        w2 += -epsilon * dw2
        b2 += -epsilon * db2

        # Assign new parameters to the model
        model = {'w1': w1, 'b1': b1, 'w2': w2, 'b2': b2}

    return model


def features(name):
    v1 = ['a', 'e', 'i', 'o', 'u']
    v2 = ['b', 'c', 'd', 'g', 'k', 'p', 'q', 't']
    n1 = 0
    n2 = 0

    name = name.lower()
    for i in range(len(name)):
        if name[i] in v1:
            n1 += 1
        if name[i] in v2:
            n2 += 1

    return [n1/len(name), n2/len(name)]

if __name__ == '__main__':

    name = []
    feature = []
    gender = []

    with open("/var/www/html/training_dataset.txt", 'r') as f:
        data = f.readlines()
        for line in data:
            odom = line.split(',')
            name.append(odom[0])
            if "female" in odom[1]:
                gender.append(1)
            else:
                gender.append(0)

    for i in range(len(name)):
        feature.append(features(name[i]))

    xarray = np.array(feature)
    yarray = np.array(gender)
    num_examples = len(xarray)

    # Train the logistic rgeression classifier
    clf = LogisticRegression()
    clf.fit(xarray, yarray)

    # Build a model with a 3-dimensional hidden layer
    model = build_model(3, xarray, yarray, num_passes=10000)

    newName = []
    newFeature = []

    with open(sys.argv[1], 'r') as f:
        data = f.readlines()
        for line in data:
            odom = line.split()
            newName.append(odom[0])

    for i in range(len(newName)):
        newFeature.append(features(newName[i]))

    N = np.array(newFeature)

    # Predict the function value for the whole gid
    Z = predict(model, N)

    for i in range(len(newName)):
        if Z[i] == 1:
            print("%s,female" % (newName[i]))
        else:
            print("%s,male" % (newName[i]))
