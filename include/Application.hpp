#include <string_view>
#include <string>

class Application {
public:
    Application(std::string_view title, int32_t width, int32_t height);

    void run();

private:
    
    void start();
    void update();

private:
    std::string title;
    int32_t width, height;

    bool running;

};
