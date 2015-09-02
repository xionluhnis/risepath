#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup()
{
    currentTool = Move;
    showHelp = showTools = true;
    ofSetFrameRate(24);
    // ofEnableAntiAliasing();
    ofEnableDepthTest(); //make sure we test depth for 3d
    ofSetVerticalSync(true);
    ofEnableLighting();
    // ofEnableAlphaBlending();
    // ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofEnableSmoothing();

    // light the scene to show off why normals are important
    light.enable();
    light.setPointLight();
    light.setPosition(0, 0, 300);
    cam.enableMouseMiddleButton();
    cam.setDistance(13.0f);

    // load the button images
    const char * files[] = {
      "eraser.png", "swirl.png", "pen.png", "pencil.png", "move.png"
    };
    ui.resize(5);
    ui[Move].state = true; // set move by default
    int m = 20;
    int dx = 0;
    for(unsigned int i = 0; i < ui.size(); ++i){
        // load image
        ui[i].data.loadImage(files[i]);
        // set position given width
        dx += ui[i].data.getWidth() + m;
        ui[i].dx = dx;
        ui[i].dy = m;
    }
}

//--------------------------------------------------------------
void ofApp::update()
{

}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofEnableLighting();
    // draw background
    ofColor centerColor = ofColor(85, 78, 68);
    ofColor edgeColor(0, 0, 0);
    ofBackgroundGradient(centerColor, edgeColor, OF_GRADIENT_CIRCULAR);

    cam.begin();
    ofSetColor(255, 255, 255, 255);

    ofDrawGrid(10.0f, 20.0f, true, false, false, true);
    // display mesh
    //mesh.draw(OF_MESH_FILL);
    // mesh.drawFaces();
    // dispMesh.draw();

    // display edges?
    /*
    dispMesh.disableColors();
    dispMesh.drawWireframe();
    dispMesh.enableColors();
    */
    // mesh.draw(OF_MESH_WIREFRAME);
    cam.end();

    // UI
    if(showTools)
    {
        for(unsigned int i = 0; i < ui.size(); ++i){
            if(ui[i].state){
                ofSetColor(100, 100, 255, 255);
            } else if(ui[i].hover){
                ofSetColor(255, 255, 255, 255);
            } else  {
                ofSetColor(255, 255, 255, 70);
            }
            ui[i].draw();
        }
    }
    if(showHelp)
    {
        ofSetColor(255, 255, 255);
        int ypos = 50;
        ofDrawBitmapString("<f> Toggle fullscreen", 50, ypos);
        ypos += 20;
        ofDrawBitmapString("<h> Toggle this help", 50, ypos);
        ypos += 20;
        ofDrawBitmapString("<1-7> Property to show", 50, ypos);
        ypos += 20;
        ofDrawBitmapString("---", 50, ypos);
        ypos += 20;
        // ofDrawBitmapString(ofFilePath::getBaseName(path), 50, ypos); ypos += 20;
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    switch(key)
    {
    case 'f':
        ofToggleFullscreen();
        break;
    case 'h':
        showHelp = !showHelp;
        break;
    case 'r':
    {
        // reset
    } break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
  for(unsigned int i = 0; i < ui.size(); ++i){
      ui[i].hover = ui[i].contains(x, y);
  }
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
  switch(currentTool){
    case Eraser:

      break;
    case Pen:

      break;
    case Pencil:

      break;
    case Swirl:

      break;
    default:
      break;
  }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
    // tool change
    Tool tool = Unknown;
    ui[currentTool].state = false;
    for(unsigned int i = 0; i < ui.size(); ++i){
      if(ui[i].hover){
          tool = static_cast<Tool>(i);
          ui[i].state = true;
          break;
      }
    }
    currentTool = tool;
    if(currentTool != Unknown){
        // switch camera off unless it's move
        if(currentTool != Move){
            cam.disableMouseInput();
            cam.disableMouseMiddleButton();
        }
        switch(tool){
            case Move:
                cam.enableMouseInput();
                cam.enableMouseMiddleButton();
                break;
            case Pen:
            case Pencil:
            case Eraser:
            case Swirl:
                // TODO show
                break;
            default:
              break;
        }
        return; // we're done
    }

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
