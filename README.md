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

- **Description**: Real-time 2D object detection video framework
- **Usage**:
  ```bash


  ```
- **Example**:
  ```bash


#### **Google_test_run**

- **Description**: Unit test using google-test
- **Usage**:
  ```bash
  # specify a test function in gtest_filter Flag
  ./Google_test_run --gtest_filter="*ImageProcessTest.TwoPassSegmentationTest*"
  ```
- **Example**:
  ```bash