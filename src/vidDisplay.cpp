#include <iostream>
#include <opencv2/opencv.hpp>
#include "../include/image_process.h" // Include the header file

using namespace cv;
using namespace std;

class CameraApp {
private:
    // Modes for image processing
    enum class Mode {
        NORMAL,         // Default mode - displays the normal image
        THRESHOLD,      // Apply threshold
        MORPHOLOGICAL   // Apply Morphological Filtering
    };

    // Constants for window names
    const string WINDOW_VIDEO = "Live";
    const string WINDOW_THRESHOLD = "HSV Thresholded";
    const string WINDOW_MORPH = "Morphological";

    // Member variables
    VideoCapture cap;           // Video capture object
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
        string suffix;
        imwrite(generateFilename(), imgs.frame);
        cout << "Saved image " << ++imageId << endl;
    }

    /**
     * @brief Applies the current filter to the captured frame based on mode
     */
    void processFrame() {
        // Always update the live video window
        resize(imgs.frame, imgs.frame, targetSize);
        imshow(WINDOW_VIDEO, imgs.frame);

        // Update threshold window if it's open
        if (currentMode == Mode::THRESHOLD || currentMode == Mode::MORPHOLOGICAL) {
            bgr_to_hsv(imgs.frame, imgs.hsv);
            extractChannel(imgs.hsv, imgs.valueChannel, 2);
            threshold(imgs.valueChannel, imgs.thresholded);
            imshow(WINDOW_THRESHOLD, imgs.thresholded);
        }

        // Update morphological window if it's open
        if (currentMode == Mode::MORPHOLOGICAL) {
            applyMorphologicalFiltering(imgs.thresholded, imgs.morp);
            imshow(WINDOW_MORPH, imgs.morp);
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
                throw runtime_error("User exit");
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
        }
    }

public:
    /**
     * @brief Constructor to initialize the CameraApp.
     * @param deviceId The ID of the camera device to use.
     */
    CameraApp(int deviceId = 0) {
        if (!cap.open("/dev/video2")) { //Replace with deviceId if needed
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
    }

    /**
     * @brief Main application loop.
     */
    void run() {
        cout << "Controls:\n"
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
                int key = waitKey(10);
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