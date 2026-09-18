class Slider {
public:
    int x, y, width, maxValue, currentValue;
    Slider(int x, int y, int width, int maxValue, int currentValue) :
        x(x), y(y), width(width), maxValue(maxValue), currentValue(currentValue) {}

    void draw(TFT_eSPI &tft) {
        int handlePosition = map(currentValue, 0, maxValue, x, x + width);
        tft.fillRect(x, y + 4, width, 8, TFT_DARKGREY); // Slider track
        tft.fillCircle(handlePosition, y + 8, 10, TFT_CYAN); // Slider handle
    }

    bool checkTouch(int tx, int ty) {
        if (tx >= x && tx <= x + width && ty >= y && ty <= y + 16) {
            currentValue = map(tx, x, x + width, 0, maxValue);
            return true;
        }
        return false;
    }
};
