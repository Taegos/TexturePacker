// TexturePacker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <vector>
#include <algorithm>
#include <iostream>
#include <stdlib.h> 

using namespace std;

namespace TexturePacker {

    struct Point {
        int x;
        int y;
        friend ostream& operator<<(ostream& os, const Point& p);
    };

    ostream& operator<<(ostream& os, const Point& p)
    {
        os << "x: " << p.x << ", " << "y: " << p.y;
        return os;
    }

    struct Size {
        int w;
        int h;
        int area() const { return w * h; };
        friend ostream& operator<<(ostream& os, const Point& p);
    };

    ostream& operator<<(ostream& os, const Size& s)
    {
        os << "w: " << s.w << ", h: " << s.h << ", a: " << s.area();
        return os;
    }


    struct Rect {

        Size size;
        Point point;

        int bottom() const {
            return point.y + size.h;
        }

        int right() const {
            return point.x + size.w;
        }

        int area() const {
            return size.area();
        }

        int space(const Rect& other) const {
            return area() - other.area();
        }

        bool fits(const Rect& other) const {

            if (other.size.w <= size.w &&
                other.size.h <= size.h) {
                return true;
            }

            return false;
        }

        friend ostream& operator<<(ostream&, const Rect&);
    };

    ostream& operator<<(ostream& os, const Rect& rect)
    {
        os << rect.point << ", " << rect.size;
        return os;
    }

    struct UserRect {
        Rect rect;
        void* data;
    };


    void pack(std::vector<UserRect>& rects) {
        if (rects.size() == 0) {
            rects[0].rect.point = { 0,0 };
            return;
        }

        std::sort(rects.begin(), rects.end(), [](const auto& a, const auto& b) {
            return a.rect.area() > b.rect.area();
        });

        UserRect largest = rects[0];
        std::vector<Rect> slots{ largest.rect };
        Size bounds = largest.rect.size;

        for (auto& usrRect : rects) {

            Rect &rect = usrRect.rect;
            int bestFitIndex = -1;

    //        cout << "Candidate: " << rect << endl;
            for (int i = 0; i < slots.size(); i++) {
                Rect candidate = slots[i];
//                cout << "Fits: " << candidate << " " << candidate.fits(rect) << endl;

                if (bestFitIndex == -1 && candidate.fits(rect)) {
                    bestFitIndex = i;
                }
                else if (bestFitIndex != -1 && candidate.fits(rect) &&
                    candidate.space(rect) < slots[bestFitIndex].space(rect)) {
                    bestFitIndex = i;
                }
            }
            if (bestFitIndex == -1) {
  //              std::cout << "none found, expand..." << std::endl;

                Size rightExpand{ bounds.w + rect.size.w, bounds.h };
                Size bottomExpand{ bounds.w, bounds.h + rect.size.h };

                Point nextPoint = { 0, bounds.h };
                Rect nextSlot { bounds.w - rect.size.w, rect.size.h, rect.right(), bounds.h };
                Size nextBounds = bottomExpand;

                int squarenessRight = abs(rightExpand.h - rightExpand.w);
                int squarenessBottom = abs(bottomExpand.h - bottomExpand.w);

                if (squarenessRight < squarenessBottom && rect.size.h <= bounds.h) {
                    nextPoint = { bounds.w, 0};
                    nextSlot = { rect.size.w, bounds.h - rect.size.h, bounds.w, rect.bottom() };
                    nextBounds = rightExpand;
//                    std::cout << "expand right: " << rect << std::endl;
                }
                else {
//                    std::cout << "expand bottom: " << rect << std::endl;
                }

                if (nextSlot.area() != 0) {
  //                  std::cout << "add expanded: " << nextSlot << std::endl;
                    slots.push_back(nextSlot);
                }

                bounds = nextBounds;
                rect.point = nextPoint;
//                std::cout << "new bounds: " << bounds << std::endl;
            }
            else {
                Rect bestFitSlot = slots[bestFitIndex];
      //          std::cout << "best fit found " << bestFitSlot << std::endl;
                rect.point = bestFitSlot.point;
                slots.erase(slots.begin() + bestFitIndex);

                Rect right{ bestFitSlot.size.w - rect.size.w, bestFitSlot.size.h, rect.right(), bestFitSlot.point.y };
                Rect bottom{ rect.size.w, bestFitSlot.size.h - rect.size.h, bestFitSlot.point.x, rect.bottom(), };
	
                if (right.area() != 0) {
    //                cout << "Add right: " << right << endl;
                    slots.push_back(right);
                }
                if (bottom.area() != 0) {
  //                  cout << "Add bottom: " << bottom << endl;
                    slots.push_back(bottom);
                }
            }
        }
    }
}

using namespace TexturePacker;

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <thread>


std::vector<UserRect> generateRects(int minD, int maxD, int count) {
    std::vector<UserRect> rects{};
    std::srand(std::time(nullptr));

    for (int i = 0; i < count; i++) {
        float rollW = std::rand() / (float)RAND_MAX;
        float rollH = std::rand() / (float)RAND_MAX;
        int diff = maxD - minD;
        int w = minD + diff * rollW;
        int h = minD + diff * rollH;
        rects.push_back({ w, h });
    }

    return rects;
}

#include <chrono>

void print(const std::vector<UserRect> rects) {

    auto maxX = *max_element(std::begin(rects), std::end(rects), [](const auto& a, const auto& b) {
        return a.rect.right() < b.rect.right();
    });

    auto maxY = *max_element(std::begin(rects), std::end(rects), [](const auto& a, const auto& b) {
        return a.rect.bottom() < b.rect.bottom();
    }); // c++11

    int sizeX = maxX.rect.right();
    int sizeY = maxY.rect.bottom();

    for (int y = 0; y < sizeY; y++) {
        std::string line(sizeX, ' ');
        for (UserRect rect : rects) {
            if (rect.rect.point.y <= y && y < rect.rect.bottom()) {
                for (int x = rect.rect.point.x; x < rect.rect.right(); x++) {
                    if (y == rect.rect.point.y || y == rect.rect.bottom() - 1) {
                        line[x] = '*';
                    }
                    else if (x == rect.rect.point.x || x == rect.rect.right() - 1) {
                        line[x] = '*';
                    }
                }
            }
        }
        cout << line << endl;
    }


}
void test1(int runs, int minD, int maxD, int rectCount) {

    float tot = 0.0;

    for (int i = 0; i < runs; i++) {
        vector<UserRect> rects = generateRects(minD, maxD, rectCount);
        auto t1 = std::chrono::high_resolution_clock::now();
        pack(rects);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        tot += duration;
    }

    std::cout << "METHOD1: avg runtime: " << tot / runs << "ms" << std::endl;
}


void test1(int runs, int minD, int maxD, int rectCount) {

    float tot = 0.0;

    for (int i = 0; i < runs; i++) {
        vector<UserRect> rects = generateRects(minD, maxD, rectCount);
        auto t1 = std::chrono::high_resolution_clock::now();
        pack(rects);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        tot += duration;
    }

    std::cout << "METHOD1: avg runtime: " << tot / runs << "ms" << std::endl;
}

int main()
{
    int runs = 1000;
    int minD = 5;
    int maxD = 10;
    int rectCount = 250;

    test1(runs, minD, maxD, rectCount);
    test2(runs, minD, maxD, rectCount);

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
