//
// Created by Yuyang Tian on 2025/1/15.
// For displaying live video with filters
//

#include <opencv2/opencv.hpp>


using namespace cv;
using namespace std;

class CameraApp {
private:
    // Modes for image processing
    enum class Mode {
        NORMAL,      // Default mode
        GREY,        // OpenCV grayscale
        BLUR,        // Apply blur
//        SOBEL_X,     // Sobel filter (X-direction)
//        SOBEL_Y,     // Sobel filter (Y-direction)
//        MAGNITUDE,   // Gradient magnitude
//        QUAN,        // Quantization
//        FACE_DECT,   // Face detection
//        FOG,         // Apply fog effect
//        BG_BLUR,     // Background blur
//        BG_GREY,     // Background grayscale
    };

    // Member variables
    VideoCapture cap;               // Video capture object
    const string OUTPUT_DIR = "../outputs/"; // Directory for saving outputs
    int imageId = 0;                // Counter for saved images
    Mode currentMode = Mode::NORMAL; // Current processing mode
    float scale_factor;             // Scaling factor for resizing
    Size targetSize;                // Target size for processed images
    const string WINDOW_VIDEO = "Live"; // Name of the video display window
    string editWindow;              // Name of the meme editor window

    // Struct to store image data
    struct Images {
        Mat frame;        // Original frame
        Mat blurred;      // Blurred frame
        Mat grey;         // Grayscale frame
        Mat processed;    // Processed frame based on mode
//        Mat sobelX;       // Sobel X component
//        Mat sobelY;       // Sobel Y component
//        Mat magnitude;    // Gradient magnitude
//        Mat quantized;    // Quantized frame
//        Rect last;        // Last detected region for smoothing
//        Mat depthMap;     // Depth map for fog effect
//        vector<Rect> faces; // Detected faces
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
     *
     * Includes logic to save different versions of the frame
     * depending on the current processing mode. Launches the
     * meme editor if applicable.
     */
    void saveImages() {
        string suffix;
        imwrite(generateFilename(), imgs.frame);
        if (currentMode != Mode::NORMAL) {
            // Generate suffix based on mode
            switch (currentMode) {
                case Mode::GREY: suffix = "-gray"; break;
//                case Mode::ALTER_GREY: suffix = "-Agray"; break;
//                case Mode::BLUR: suffix = "-blurred"; break;
//                case Mode::SOBEL_X: suffix = "-sobelX"; break;
//                case Mode::SOBEL_Y: suffix = "-sobelY"; break;
//                case Mode::MAGNITUDE: suffix = "-magnitude"; break;
//                case Mode::QUAN: suffix = "-quantization"; break;
//                case Mode::FACE_DECT: suffix = "-faceDetect"; break;
//                case Mode::FOG: suffix = "-fog"; break;
//                case Mode::BG_BLUR: suffix = "-bgBlur"; break;
//                case Mode::BG_GREY: suffix = "-bgGrey"; break;
                default: break;
            }
            imwrite(generateFilename("Task", suffix), imgs.processed);
        }
        cout << "Saved image " << ++imageId << endl;
    }

    /**
     * @brief Applies the current filter to the captured frame.
     */
    void processFrame() {
        switch (currentMode) {
            case Mode::GREY:
                cvtColor(imgs.frame, imgs.processed, COLOR_BGR2GRAY);
                break;
            case Mode::BLUR:
//                blur5x5_2(imgs.frame, imgs.processed);
                break;
            default:
                imgs.frame.copyTo(imgs.processed);
                break;
        }
        resize(imgs.processed, imgs.processed, targetSize);
        imshow(WINDOW_VIDEO, imgs.processed);
    }

    /**
     * @brief Routes keypresses to the appropriate handler.
     * @param key The pressed key.
     */
    void handleKeyPress(char key) {
        switch (key) {
            case 's': saveImages(); break;
            case 'q': throw runtime_error("User exit");
        }
    }

public:
    /**
     * @brief Constructor to initialize the CameraApp.
     * @param deviceId The ID of the camera device to use.
     */
    CameraApp(int deviceId = 0) {
        if (!cap.open(deviceId)) {
            throw runtime_error("Failed to open camera device " + to_string(deviceId));
        }

        Size frameSize(cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));
        const float reduction = 0.8;
        scale_factor = 256.0 / (frameSize.height * reduction);

        targetSize.width = frameSize.width * scale_factor;
        targetSize.height = frameSize.height * scale_factor;

        cout << "Camera initialized with resolution: " << targetSize.width << "x" << targetSize.height << endl;
    }

    /**
     * @brief Main application loop.
     *
     * Continuously captures video frames, applies processing,
     * and responds to user input until the application exits.
     */
    void run() {
        cout << "Controls:\n"
             << "  's' - Save photo\n"
             << "  'g' - Toggle OpenCV grayscale\n"
             << "  'h' - Toggle custom grayscale\n"
             << "  'b' - Toggle blur\n"
             << "  'x' - Toggle Sobel X\n"
             << "  'y' - Toggle Sobel Y\n"
             << "  'm' - Toggle Magnitude\n"
             << "  'f' - Toggle face detection\n"
             << "  'o' - Toggle fog effect\n"
             << "  'q' - Quit\n";

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
