"""
/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/16
 * Purpose: Train a Decision tree using sklearn decision tree
 * MANDATORY: run it to generate a new decision tree and
 *            replace the `classifyByDecisionTree` function in `classifiers.cpp` when the training data changes
 *
 */
"""

from pymongo.mongo_client import MongoClient
from pymongo.server_api import ServerApi
import numpy as np
from sklearn.tree import DecisionTreeClassifier, _tree
# NOTE: pip install pymongo scikit-learn numpy please running this program
# Connect to MongoDB
uri =  "mongodb+srv://yuyangtian23:Woaiyangyang0810!@cs5330-proj3.mvlhe.mongodb.net/?retryWrites=true&w=majority&appName=CS5330-Proj3";
# Create a new client and connect to the server
client = MongoClient(uri, server_api=ServerApi('1'))
# Send a ping to confirm a successful connection
try:
    client.admin.command('ping')
    print("Pinged your deployment. You successfully connected to MongoDB!")
except Exception as e:
    print(e)
db = client["feature_db"]  # Replace with your DB name
collection = db["features"]  # Replace with your collection name

# Fetch training data
cursor = collection.find({}, {"features": 1, "type": 1})  # Adjust query as needed

X = []
y = []
label_map = {}  # To map string labels to numbers

# Load data from DB
for doc in cursor:
    features = doc["features"]
    label = doc["type"]

    if label not in label_map:
        label_map[label] = len(label_map)  # Assign an integer ID to each class

    X.append(features)
    y.append(label_map[label])

X = np.array(X)
y = np.array(y)

# Train a decision tree with max depth 3
clf = DecisionTreeClassifier(max_depth=3)
clf.fit(X, y)
print("feature importance: ", clf.feature_importances_);


# Convert Decision Tree to If-Else Statements for C++
def tree_to_if_else(tree, feature_names, label_map):
    tree_ = tree.tree_
    labels = {v: k for k, v in label_map.items()}  # Reverse mapping (int -> label)

    def recurse(node, depth):
        indent = "    " * depth
        if tree_.feature[node] != _tree.TREE_UNDEFINED:
            name = feature_names[tree_.feature[node]]
            threshold = tree_.threshold[node]
            print(f"{indent}if ({name} <= {threshold:.6f}) {{")
            recurse(tree_.children_left[node], depth + 1)
            print(f"{indent}}} else {{")
            recurse(tree_.children_right[node], depth + 1)
            print(f"{indent}}}")
        else:
            class_label = int(tree_.value[node][0].argmax())  # Predicted class index
            label_name = labels[class_label]  # Get the actual class name
            print(f'{indent}return "{label_name}";')  # Return class name

    print('std::string classify(float features[]) {')
    recurse(0, 1)
    print("}")

# Generate the C++ classification function
tree_to_if_else(clf, [f"features[{i}]" for i in range(X.shape[1])], label_map)



