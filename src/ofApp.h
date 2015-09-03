#pragma once

#include "ofMain.h"
#include "camera/ofStableCam.h"

struct ofButton {
  ofImage data;
  int dx, dy;
  bool hover, state;

  void draw(){
      data.draw(ofGetViewportWidth() - dx, dy);
  }

  bool contains(int mx, int my) {
      int x = ofGetViewportWidth() - dx;
      int y = dy;
      return mx >= x && mx < x + data.getWidth()
          && my >= y && my < y + data.getHeight();
  }
};

enum Tool {
    Eraser  = 0,
    Swirl   = 1,
    Pen     = 2,
    Pencil  = 3,
    Move    = 4,
    Unknown = 5
};

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		void drawUI();
		void drawGrid();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		ofVec3f mouseOnPlane();

		void selectTool(Tool tool);
		float getCellSize() const;

    private:
        // World objects
        ofStableCam cam;
        ofLight light;

        // Parameters
        Tool currentTool;
        bool showHelp, showTools;
        unsigned int resolveLevel;
        float currentZ, startZ, lastZ;

        // UI
        std::vector<ofButton> ui;
        bool dragLevel;
        int startX, startY;
        int lastX, lastY;
};
