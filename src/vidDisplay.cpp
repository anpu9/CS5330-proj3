#include <iostream>
#include <opencv2/opencv.hpp>
#include "../include/image_process.h"
#include "../include/obb_feature_extraction.h"
#include "../db/db_manager.h"

using namespace cv;
using namespace std;

class CameraApp {
private:
    // Modes for image processing
    enum class Mode {
        NORMAL,         // Default mode - displays the normal image
        THRESHOLD,      // Apply threshold
        MORPHOLOGICAL,   // Apply Morphological Filtering
        OBB, // apply all threshold, Morphological Filtering, segmentation, OBB computation
    };

    // Constants for window names
    const string WINDOW_VIDEO = "Live";
    const string WINDOW_THRESHOLD = "HSV Thresholded";
    const string WINDOW_MORPH = "Morphological";
    const string WINDOW_OBB = "OBB over image";

    // Member variables
    VideoCapture cap;           // Video capture object
    DBManager db;

    const string OUTPUT_DIR = "../outputs/";  // Directory for saving outputs
    int imageId = 0;            // Counter for saved images

    Mode currentMode = Mode::NORMAL;   // Current processing mode

    float scale_factor;         // Scaling factor for resizing
    Size targetSize;            // Target size for processed images



    // Struct to store image data
    struct Images {
        Mat frame;          // Original frame
        Mat hsv;            // HSV frame
        Mat valueChannel;   // Value channel of HSV
        Mat thresholded;    // Thresholded image
        Mat morp;           // Morphological filtered image
        Mat regionMap;      // Store segment region id
        Mat obb;            // Original frame with OBB overlay
        vector<float> features;     // Region features of the current object
    } imgs;

    /**
     * @brief Generates a filename for saving images.
     * @param task Optional prefix for the task.
     * @param suffix Optional suffix for file description.
     * @return A string representing the generated filename.
     */
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
     * @brief Applies the current filter to the captured frame based on mode
     */
    void processFrame() {
        // Always update the live video window
        resize(imgs.frame, imgs.frame, targetSize);
        imshow(WINDOW_VIDEO, imgs.frame);

        // Update threshold window if it's open
        // Convert to HSV and extract value channel if any dependent mode is active
        bool needsThresholding = (currentMode == Mode::THRESHOLD ||
                                  currentMode == Mode::MORPHOLOGICAL ||
                                  currentMode == Mode::OBB);

        if (needsThresholding) {
            bgr_to_hsv(imgs.frame, imgs.hsv);
            extractChannel(imgs.hsv, imgs.valueChannel, 2);
            threshold(imgs.valueChannel, imgs.thresholded);
        }

        // Show threshold window if needed
        if (currentMode == Mode::THRESHOLD || currentMode == Mode::MORPHOLOGICAL) {
            imshow(WINDOW_THRESHOLD, imgs.thresholded);
        }

        // Apply morphological filtering if required
        if (currentMode == Mode::MORPHOLOGICAL || currentMode == Mode::OBB) {
            applyMorphologicalFiltering(imgs.thresholded, imgs.morp);
        }

        // Show morphological window if in MORPHOLOGICAL mode
        if (currentMode == Mode::MORPHOLOGICAL) {
            imshow(WINDOW_MORPH, imgs.morp);
        }

        // Process OBB imgs.features if in OBB mode
        if (currentMode == Mode::OBB) {
            twoPassSegmentation8conn(imgs.morp, imgs.regionMap);
            computeRegionFeatures(imgs.regionMap, 0, imgs.frame, imgs.obb, imgs.features);
            imshow(WINDOW_OBB, imgs.obb);
        }
    }

    /**
     * @brief Routes keypresses to the appropriate handler.
     * @param key The pressed key.
     */
    void handleKeyPress(char key) {
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
                if (currentMode != Mode::THRESHOLD) {
                    currentMode = Mode::THRESHOLD;
                    namedWindow(WINDOW_THRESHOLD, WINDOW_AUTOSIZE);
                } else {
                    currentMode = Mode::NORMAL;
                    destroyWindow(WINDOW_THRESHOLD);
                }
                break;
            case 'f':
                if (currentMode == Mode::THRESHOLD) {
                    currentMode = Mode::MORPHOLOGICAL;
                    namedWindow(WINDOW_MORPH, WINDOW_AUTOSIZE);
                } else if (currentMode == Mode::MORPHOLOGICAL) {
                    currentMode = Mode::THRESHOLD;
                    destroyWindow(WINDOW_MORPH);
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
                if (currentMode == Mode::OBB) {
                    string label;
                    cout << "Please enter the feature for the object: ";
                    cin >> label;
                    db.writeFeatureVector(label, imgs.features);
                }
        }
    }

public:
    /**
     * @brief Constructor to initialize the CameraApp.
     * @param deviceId The ID of the camera device to use.
     */
    CameraApp(int deviceId = 0) {
        if (!cap.open(deviceId)) { //Replace with deviceId if needed
            throw runtime_error("Failed to open camera device " + to_string(deviceId));
        }

        Size frameSize(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
        const float reduction = 0.8;
        scale_factor = 256.0 / (frameSize.height * reduction);
        targetSize.width = frameSize.width * scale_factor;
        targetSize.height = frameSize.height * scale_factor;
        cout << "Camera initialized with resolution: " << targetSize.width << "x" << targetSize.height
             << endl;

        // Initialize main window
        namedWindow(WINDOW_VIDEO, WINDOW_AUTOSIZE);
        // Initialize DB
        db = DBManager();
    }

    /**
     * @brief Main application loop.
     */
    void run() {
        cout << "Controls:\n"
        // TODO: write into new features
             << " 't' - Toggle traning mode\n"
             << " 'n' - add new features within traning mode\n"
             << " 's' - Save photo\n"
             << " 'z' - Toggle Thresholding\n"
             << " 'f' - Toggle Morphological window\n"
             << " 'q' - Quit\n";
        try {
            while (true) {
                if (!cap.read(imgs.frame)) {
                    throw runtime_error("Failed to capture frame");
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

int main() {
    try {
        CameraApp app(0);  // Use 0 for external camera, 1 for default camera
        app.run();
        return 0;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -1;
    }
}