/*
 * Authors: Yuyang Tian and Arun Mekkad
 * Date: 2025/2/12
 * Purpose: Implementation of real-time object detection
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include "../include/image_process.h"
#include "../include/obb_feature_extraction.h"
#include "../db/db_manager.h"
#include "../include/classifier.h"
#include "../include/evaluate.h"

using namespace cv;
using namespace std;

class CameraApp {
private:
    // Modes for image processing
    enum class Mode {
        NORMAL,
        THRESHOLD,
        MORPHOLOGICAL,
        COLOR_SEG,
        OBB,
    };
    enum class CLASSIFIER {
        NN,
        DT,
    };

    // Constants for window names
    const string WINDOW_VIDEO = "Live";
    const string WINDOW_THRESHOLD = "HSV Thresholded";
    const string WINDOW_MORPH = "Morphological";
    const string WINDOW_OBB = "OBB over image";
    const string WINDOW_SEG = "Colored segmentation - 8 connectivity";

    // Member variables
    VideoCapture cap;
    DBManager db;
    vector<pair<string, vector<float>>> dbFeatures; // Static storage
    const string OUTPUT_DIR = "../outputs/";
    int imageId = 0;
    Mode currentMode = Mode::NORMAL;
    CLASSIFIER currentClassifier = CLASSIFIER::NN;
    float scale_factor;
    Size targetSize;
    bool trainingMode;
    struct Images {
        Mat frame;
        Mat hsv;
        Mat valueChannel;
        Mat thresholded;
        Mat morp;
        Mat regionMap;
        Mat colorizedRegions;
        Mat obb;
        vector<float> features;
    } imgs;

    string generateFilename(const string& task = "", const string& suffix = "") {
        return OUTPUT_DIR + task + to_string(imageId) + suffix + ".png";
    }
    /**
     * @brief Saves the current frame and processed frame to disk.
     */
    void saveImages() {
        imwrite(generateFilename(), imgs.frame);
        cout << "Saved normal image " << ++imageId << endl;
        // Save other frames if they exist
        switch (currentMode) {
            case Mode::MORPHOLOGICAL:
                imwrite(generateFilename("Task1", "THRESHOLD"), imgs.thresholded);
                cout << "Saved thresholded image " << ++imageId << endl;
                imwrite(generateFilename("Task2", "MORPHOLOGICAL"), imgs.morp);
                cout << "Saved morp image " << ++imageId << endl;
                break;
            case Mode::THRESHOLD:
                imwrite(generateFilename("Task1", "THRESHOLD"), imgs.thresholded);
                cout << "Saved thresholded image " << ++imageId << endl;
                break;
            default:
                // No additional images to save
                break;
        }
    }
    /**
     * @brief Classifies the current object using the specified classifier.
     * @param currentFeatures The feature vector of the current object.
     * @param classifier The classifier to use (NN or Decision Tree).
     * @return The label of the closest object in the database.
     */

    string classifyObject(const vector<float>& currentFeatures, CLASSIFIER classifier) {
        if (dbFeatures.empty()) {
            db.loadFeatureVectors(dbFeatures); // Load features only once
        }
        if (dbFeatures.empty()) {
            return "Unknown";
        }
        string closestLabel = "Unknown";
        switch (classifier) {
            case CLASSIFIER::NN:
                closestLabel = classifyByNN(dbFeatures, currentFeatures);
                break;
            case CLASSIFIER::DT:
                closestLabel = classifyByDecisionTree(currentFeatures);
                break;
            default:
                cerr << "ERROR: it's not a valid classifier!";
                exit(-1);
        }
        return closestLabel;
    }

// Applies image processing and classification respective to the current mode
    void processFrame() {
        resize(imgs.frame, imgs.frame, targetSize);
        if (trainingMode) {
            imshow(WINDOW_VIDEO, imgs.frame);
        }

        if (currentMode >= Mode::THRESHOLD) {
            bgr_to_hsv(imgs.frame, imgs.hsv);
            extractChannel(imgs.hsv, imgs.valueChannel, 2);
            threshold(imgs.valueChannel, imgs.thresholded);
            if(trainingMode)
            imshow(WINDOW_THRESHOLD, imgs.thresholded);
        }

        if (currentMode >= Mode::MORPHOLOGICAL) {
            applyMorphologicalFiltering(imgs.thresholded, imgs.morp);
            if(trainingMode)
            imshow(WINDOW_MORPH, imgs.morp);
        }

        if (currentMode >= Mode::COLOR_SEG) {
            twoPassSegmentation8conn(imgs.morp, imgs.regionMap);
            colorizeRegions(imgs.regionMap, imgs.colorizedRegions);
            if(trainingMode)
            imshow(WINDOW_SEG, imgs.colorizedRegions);
        }

        if (currentMode == Mode::OBB || !trainingMode) {
            computeRegionFeatures(imgs.regionMap, 0, imgs.frame, imgs.obb, imgs.features);
            if (trainingMode) {
                imshow(WINDOW_OBB, imgs.obb);
            } else {
                string label = classifyObject(imgs.features, currentClassifier);
                putText(imgs.obb, label, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 127, 80), 2);
                imshow(WINDOW_OBB, imgs.obb);
            }
        }
        if (!trainingMode) {
            // Classifier display
            switch (currentClassifier) {
                case CLASSIFIER::NN:
                    putText(imgs.obb, "Nearest neighbor", Point(400, 300), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(147, 112, 219), 1);
                    imshow(WINDOW_OBB, imgs.obb);
                    break;
                case CLASSIFIER::DT:
                    putText(imgs.obb, "Decison tree", Point(400, 300), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(147, 112, 219), 1);
                    imshow(WINDOW_OBB, imgs.obb);
                    break;
            }
        }
    }
    /**
     * @brief Routes keypresses to the appropriate handler.
     * @param key The pressed key.
     */
    void handleKeyPress(char key) {
        if (trainingMode) {
            switch (key) {
                    case 's':
                        saveImages();
                        break;
                    case 'q':
                        cout << "Quitting..." << endl;
                        currentMode = Mode::NORMAL;  // Reset mode
                        destroyAllWindows();  // Close all windows
                        exit(0);
                        break;
                    case 'z':
                        if (currentMode < Mode::THRESHOLD) {
                            currentMode = Mode::THRESHOLD;
                            namedWindow(WINDOW_THRESHOLD, WINDOW_AUTOSIZE);
                        } else {
                            currentMode = Mode::NORMAL;
                            destroyWindow(WINDOW_THRESHOLD);
                        }
                        break;
                    case 'f':
                        if (currentMode < Mode::MORPHOLOGICAL) {
                            currentMode = Mode::MORPHOLOGICAL;
                            namedWindow(WINDOW_MORPH, WINDOW_AUTOSIZE);
                        } else if (currentMode > Mode::MORPHOLOGICAL) {
                            currentMode = Mode::MORPHOLOGICAL;
                            destroyWindow(WINDOW_SEG);
                        }
                        break;
                    case 'c':
                        if (currentMode < Mode::COLOR_SEG) {
                            currentMode = Mode::COLOR_SEG;
                            namedWindow(WINDOW_SEG, WINDOW_AUTOSIZE);
                        } else {
                            currentMode = Mode::MORPHOLOGICAL;
                            destroyWindow(WINDOW_SEG);
                        }
                        break;
                    case 't':
                        if (currentMode == Mode::OBB) {
                            currentMode = Mode::NORMAL;
                            destroyWindow(WINDOW_OBB);  // Close OBB window when exiting
                        } else {
                            currentMode = Mode::OBB;
                            namedWindow(WINDOW_OBB, WINDOW_AUTOSIZE);  // Recreate window if needed
                        }
                        break;
                    case 'n':
                        if (currentMode == Mode::OBB ) {
                            string label;
                            cout << "Please enter the type label for the object: ";
                            getline(cin, label);  // Read entire line
                            // Trim trailing spaces
                            size_t end = label.find_last_not_of(" \t\r\n");
                            if (end != string::npos) {
                                label = label.substr(0, end + 1);
                            }

                            db.writeFeatureVector(label, imgs.features);
                        }
                        break;
                }
        } else {
            switch (key) {
                case 'n':
                    currentClassifier = CLASSIFIER::NN;
                    cout << "using NN" << endl;

                    break;
                case 'd':
                    currentClassifier = CLASSIFIER::DT;
                    cout << "using DT" << endl;
                    break;
                case 'q':
                    cout << "Quitting..." << endl;
                    currentMode = Mode::NORMAL;  // Reset mode
                    destroyAllWindows();  // Close all windows
                    exit(0);
                    break;
                case 'e':
                    if(currentClassifier == CLASSIFIER::NN) {
                        evaluateConfusionMatrix(0);
                    } else {
                        evaluateConfusionMatrix(1);
                    }
            }
        }
    }

public:
    /**
     * @brief Constructor to initialize the CameraApp.
     * @param deviceId The ID of the camera device to use.
     */
    CameraApp(int deviceId = 0, bool trainingMode = false) : trainingMode(trainingMode) {
        if (!cap.open(deviceId)) {
            throw runtime_error("Failed to open camera device " + to_string(deviceId));
        }

        Size frameSize(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
        const float reduction = 0.8;
        scale_factor = 256.0 / (frameSize.height * reduction);
        targetSize.width = frameSize.width * scale_factor;
        targetSize.height = frameSize.height * scale_factor;
        cout << "Camera initialized with resolution: " << targetSize.width << "x" << targetSize.height << endl;

        namedWindow(WINDOW_VIDEO, WINDOW_AUTOSIZE);
        db = DBManager();

        if (!trainingMode) {
            currentMode = Mode::OBB;
            namedWindow(WINDOW_OBB, WINDOW_AUTOSIZE);
            destroyWindow(WINDOW_VIDEO);
        }
    }

    /**
     * @brief Main application loop.
     */
    void run() {
        if (trainingMode) {
            cout << "Training Mode:\n"
                 << " 's' - Save photo\n"
                 << " 'z' - Toggle Thresholding\n"
                 << " 'f' - Toggle Morphological window\n"
                 << " 'c' - Toggle Colored segmented region window\n"
                 << " 't' - Toggle traning mode\n"
                 << " 'n' - add new features within training mode\n"
                 << " 'q' - Quit\n";
        } else {
            cout << "Classification Mode: DEFAULT classifier - Nearest neighbor\n"
                 << " 'n' - Nearest neighbor \n"
                 << " 'd' - Decision tree \n"
                 << " 'q' - Quit\n";
        }
        try {
            while (true) {
                if (!cap.read(imgs.frame)) {
                    throw runtime_error("Failed to capture frame");
                }
                if (imgs.frame.empty()) {
                    cerr << "Error: Image not loaded!" << endl;
                    return;
                }
                processFrame();
                int key = waitKey(30);
                if (key != -1) {
                    handleKeyPress(static_cast<char>(key));
                }
            }
        } catch (const runtime_error& e) {
            cout << "Exiting: " << e.what() << endl;
        }
    }

    ~CameraApp() {
        cap.release();
        destroyAllWindows();
    }
};

int main(int argc, char* argv[]) {
    try {
        bool trainingMode = false;
        if (argc > 1 && string(argv[1]) == "--train") {
            trainingMode = true;
        }
        CameraApp app(2, trainingMode); // Pass trainingMode to constructor
        app.run();
        return 0;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -1;
    }
}
