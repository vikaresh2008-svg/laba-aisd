#include <iostream>
#include <exception>
#include "Image.hpp"

int main() {
    try {
        Image<short> img1(5, 5, true);
        Image<short> img2(5, 5, true);

        std::cout << "Image 1:\n" << img1 << "\n";
        std::cout << "Image 2:\n" << img2 << "\n";

        std::cout << "img1 == img2: " << (img1 == img2 ? "true" : "false") << "\n";

        Image<short> sumImg = img1 + img2;
        std::cout << "Image 1 + Image 2:\n" << sumImg << "\n";

        Image<float> canvas(10, 10, false);
        drawFilledCircle(canvas, 4, 4, 3, 1.0f);
        std::cout << "Canvas with filled circle:\n" << canvas << "\n";

    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
    return 0;
}