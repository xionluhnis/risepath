#pragma once

#include "ofMain.h"

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
    Pen     = 1,
    Pencil  = 2,
    Move    = 3,
    Swirl   = 4,
    Unknown = 5
};

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

    private:
        ofEasyCam cam;
        ofLight light;

        // UI
        Tool currentTool;
        bool showHelp, showTools;
        std::vector<ofButton> ui;
};
