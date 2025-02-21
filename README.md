# 🚀 Proj3

## 📄 Project Report: [🔗 Link](https://docs.google.com/document/d/1ebXFYiskRRMnh7r--UabbHMDvGZ-x7tpl08yb7g5h3o/edit?tab=t.0)

## 👥 Team Members
1. **Yuyang Tian**
2. **Arun Mekkad**

## 💻 Environment
- **🖥️ Yuyang Tian**: macOS 10.13.1 + CLion + CMake
- **🐧 Arun Mekkad**: Ubuntu 22.04 LTS + VS Code + CMake

## 📌 Instructions for Running Executables

### 📂 File Structure (in progress)
```bash
Proj3/
   ├── include/              # 📁 Header files
   ├── src/                  # 📁 Source files - most of them are executables.
   ├── tests/                # 🧪 Testing C++ executables 
   ├── Google_tests/         # 📚 Google testing libraries
   ├── test-imgs/            # 🖼️ Image set for testing
   ├── CMakeLists.txt        # ⚙️ CMake build configuration
   ├── README.md             # 📖 Project documentation
```

### 🏃‍️Executables
#### **VidDisplay**

- **Description**: Real-time 2D Object Detection
- **Usage**:
  ```bash
  Object Detection Mode: ./VidDisplay
  
  Training mode: ./VidDisplay --train
  ```
- **Example**:
  ```bash
  The following key bindings work only on training mode
  Task 1 - Press 'z' to display thresholded window (BGR -> HSV -> k-means algorithm)
  Task 2 - Press 'f' after 'z' to display morphological filter window
  Task 3 - Press 'c' to view the segmented regions
  Task 4 - Press 't' into training mode
  Task 5 - Press 'n' after 't' to create a new feature vector into DB under training mode
  Task 6 - Press 'n' to have Nearest Neighbour as classifier, its also default under Object
           Detection mode
  Task 7 - Press 'e' to evaluate confusion matrix when not run on training mode for 
           respective classifiers
  Task 9 - Press 'd' to have Sklearn Decision tree as classifier under Object Detection Mode
           NOTE: please re-run src/decision_tree.py to generate a new decision tree after training change. And replace the decision tree logic in src/classifier.cpp


#### **Google_test_run**

- **Description**: Unit test using google-test
- **Usage**:
  ```bash
  # specify a test function in gtest_filter Flag
  ./Google_test_run --gtest_filter="*[TestSuiteName].[TestName]*"
  ```
- **Example**:
  ```bash
  
  ./Google_test_run --gtest_filter="*ImageProcessingTest.TwoPassSegmentationTest*"
  ```