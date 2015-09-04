#include "ofApp.h"

#include <openvdb/tools/Prune.h>
// #include <gtk/gtk.h>
#include <cstdlib>

#include <iostream>
#define MAX_RESOLUTION    5
#define MODE_Z_KEY        2304
#define MODE_Z_KEY_LEFT   2305
#define MODE_Z_KEY_RIGHT  2306
#define CTRL_KEY          768
#define CTRL_KEY_LEFT     769
#define CTRL_KEY_RIGHT    770

#include "rays/Plane.h"
#include "rays/Ray.h"

static float round(float f) {
  return std::floor(f + 0.5f);
}
static ofRectangle resolveSlider() {
    int x = ofGetViewportWidth() - 20 - 48;
    int w = 32;
    int y = 20 + 64 + 20 + 20;
    int h = ofGetViewportHeight() - y - 20;
    return ofRectangle(x, y, w, h);
}
//--------------------------------------------------------------
void ofApp::setup()
{
    std::cout << "Initializing OpenVDB.\n";
    openvdb::initialize();
    grid = ovdbGrid::create();
    // set world transformation
    voxelTransform = ovdbXForm::createLinearTransform(/*voxel size=*/ 1.0 / double(1 << 5));
    grid->setTransform(voxelTransform);
    // potentially load file
    const char *str = std::getenv("FILE");
    if(str){
        bool exists = ofFile(ofFilePath::join("..", fname)).exists();
        std::cout << "FILE=" << str << "(exists=" << exists << ")" << std::endl;
        string fname(str);
        if(exists && fname.substr(fname.size() - 4) == ".vdb"){
            loadFile(fname);
        }
    } else std::cout << "No FILE.\n";


    std::cout << "Initializing GL Context.\n";
    currentTool = Move;
    showHelp = showTools = true;
    ctrl = shift = alt = false;
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

    std::cout << "Initializing UI.\n";
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

// @see https://github.com/elliotwoods/ofxRay
// @see http://forum.openframeworks.cc/t/how-to-i-use-ofcamera-screentoworld-to-calculate-positon-on-a-plane/14828
ofVec3f ofApp::mouseOnPlane() {
    ofVec3f screenToWorld = cam.screenToWorld(ofVec3f(ofGetMouseX(), ofGetMouseY(), 0.0f));
    ofVec3f rayFrom = cam.getPosition();
    ofVec3f rayDir  = screenToWorld - rayFrom;
    ofxRay::Plane plane(ofVec3f(0.0f, 0.0f, currentZ), ofVec3f(0.0f, 0.0f, 1.0f));
    ofVec3f pointOnPlane;
    plane.intersect(ofxRay::Ray(rayFrom, rayDir), pointOnPlane);
    return pointOnPlane;
}

//--------------------------------------------------------------
void ofApp::update()
{
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofEnableLighting();
    ofSetColor(255, 255, 255, 255);

    // draw background
    ofColor centerColor = ofColor(85, 78, 68);
    ofColor edgeColor(0, 0, 0);
    ofBackgroundGradient(centerColor, edgeColor, OF_GRADIENT_CIRCULAR);

    drawGrid();
    drawUI();
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
    case 't':
        selectTool(static_cast<Tool>((currentTool + static_cast<int>(Unknown) - 1) % static_cast<int>(Unknown))); // next
        break;
    case 'T':
        selectTool(static_cast<Tool>((currentTool + 1) % static_cast<int>(Unknown))); // prev
        break;
    case 'r':
        cam.reset();
      break;
    case MODE_Z_KEY:
    case MODE_Z_KEY_LEFT:
    case MODE_Z_KEY_RIGHT:
        if(currentTool == Move){
            cam.disableMouseMiddleButton();
        }
        break;
    case CTRL_KEY:
    case CTRL_KEY_LEFT:
    case CTRL_KEY_RIGHT:
        ctrl |= key;
        break;
    case ' ': {
      /* THIS WON'T WORK SINCE GTK LOOP NOT ACCESSIBLE even with gtk_init => gtk_main stuck
        string fname;
        GtkWidget *dialog = gtk_file_chooser_dialog_new ("Open File",
                      NULL,
                      GTK_FILE_CHOOSER_ACTION_OPEN,
                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                      NULL);
        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
            fname = filename;
            g_free (filename);
          }
        gtk_widget_hide (dialog);

        if(fname.substr(fname.size() - 4) == ".vdb"){
            // merge grid content
            loadFile(fname);
        } else {
            std::cout << "Unsupported file.\n";
        }
        int argc = 0;
        char **argv = NULL;
        gtk_init(&argc, &argv);
        */
        } break;
    default:
        std::cout << "key '" << key << "' pressed.\n";
        break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
    switch(key)
    {
    case MODE_Z_KEY:
    case MODE_Z_KEY_LEFT:
    case MODE_Z_KEY_RIGHT:
        if(currentTool == Move){
            cam.enableMouseMiddleButton();
        }
        break;
    case CTRL_KEY:
    case CTRL_KEY_LEFT:
    case CTRL_KEY_RIGHT:
        ctrl ^= ctrl & key; // remove mask (the part of it that exjsts in ctrl)
        break;
    case 's':
        if (ctrl){
            if(grid->activeVoxelCount()){
                std::cout << "Saving current volume to risepath.vdb\n";
                openvdb::io::File file("risepath.vdb");
                openvdb::GridPtrVec grids;
                grids.push_back(grid);
                file.write(grids);
                file.close();
            } else std::cout << "No voxel to save.\n";
        } else std::cout << "Save with <Ctrl>.\n";
        break;
    case 'n':
        if(ctrl){
            grid->clear(); // delete grid content
        }
        break;
    default:
        break;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
    for(unsigned int i = 0; i < ui.size(); ++i){
        ui[i].hover = ui[i].contains(x, y);
    }
}

//--------------------------------------------------------------
static void updateVoxels(ovdbGridPtr grid, const ofVec3f &p, int level, bool value)
{

    ovdbCoord center;
    for(int i = 0; i < 3; ++i)
        center[i] = std::floor(p[i] * (1 << 5)) ;
    int dim = level == 5 ? 0 : (1 << (5 - level - 1));
    ovdbCoordBBox bbox(center.offsetBy(-dim, -dim, 0),
                            center.offsetBy(+dim, +dim, 2 * dim));
    grid->fill(bbox, value, value);
    std::cout << "Fill: " << bbox << " with " << value << " @" << p << std::endl;
    /*
    openvdb::Coord ijk;
    int &i = ijk[0], &j = ijk[1], &k = ijk[2];
    for (i = c[0] - dim; i < c[0] + dim; ++i) {
        for (j = c[1] - dim; j < c[1] + dim; ++j) {
            for (k = c[2] - dim; k < c[2] + dim; ++k) {
                // Set the distance for voxel (i,j,k).
                voxels.setValue(ijk, value);
            }
        }
    }
    */
}

void ofApp::mouseDragged(int x, int y, int button)
{
    // std::cout << "mouseDragged: " << x << ", " << y << " button: " << button << std::endl;
    if(dragLevel){
        ofRectangle rect = resolveSlider();
        rect.scaleFromCenter(0.7f, 0.99f);
        resolveLevel = std::max(0, std::min(MAX_RESOLUTION, (int)std::floor(0.5f + MAX_RESOLUTION * (y - rect.y) / std::max(1.0f, rect.height)) ));
        return;
    }
    if(button == OF_MOUSE_BUTTON_MIDDLE && (currentTool != Move || ofGetKeyPressed(MODE_Z_KEY))){
        int dy = y - startY;
        float dh = -dy * 3.0f / ofGetViewportHeight();
        float delta = getCellSize();
        lastZ = currentZ;
        currentZ = std::min(10.0f, std::max(0.0f, std::floor((startZ + dh) / delta) * delta));
        std::cout << "z from " << lastZ << " to " << currentZ << ", delta=" << delta << ", dh=" << dh << std::endl;
        return;
    }
    ofVec3f target;
    if(currentTool != Move){
      ofVec3f pos = mouseOnPlane();
      float cellSize = getCellSize();
      float cellRadius = cellSize * 0.5f;
      float voxX = std::floor(pos.x / cellSize) * cellSize;
      float voxY = std::floor(pos.y / cellSize) * cellSize;
      target = ofVec3f(voxX + cellRadius, voxY + cellRadius, currentZ);
    }
    switch(currentTool){
      case Eraser:
        updateVoxels(grid, target, resolveLevel, false);
        openvdb::tools::pruneInactive(grid->tree());
        break;
      case Pen:

        break;
      case Pencil:
        updateVoxels(grid, target, resolveLevel, true);
        openvdb::tools::prune(grid->tree());
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
    // std::cout << "mousePressed: " << x << ", " << y << " button: " << button << std::endl;
    // tool change
    Tool tool = Unknown;
    for(unsigned int i = 0; i < ui.size(); ++i){
      if(ui[i].hover){
          tool = static_cast<Tool>(i);
          break;
      }
    }
    if(tool != Unknown){
        selectTool(tool);
        return;
    }
    // potential drags
    startX = lastX = x;
    startY = lastY = y;
    startZ = lastZ = currentZ;
    // slider drag?
    ofRectangle rect = resolveSlider();
    if(rect.inside(x, y)){
        dragLevel = true;
        if(currentTool == Move){
            cam.disableMouseInput();
            cam.disableMouseMiddleButton();
        }
    } else if(button != OF_MOUSE_BUTTON_MIDDLE && tool != Move){
        mouseDragged(x, y, button);
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
    // std::cout << "mouseReleased: " << x << ", " << y << " button: " << button << std::endl;
    // std::cout << "dragLevel=" << dragLevel << ", currentTool=" << currentTool << "\n";
    if(dragLevel && currentTool == Move){
        cam.enableMouseInput();
        cam.enableMouseMiddleButton();
    }

    // no more dragging
    dragLevel = false;
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
    // load vdb grids
    for(unsigned int i = 0; i < dragInfo.files.size(); ++i){
        const string &fname = dragInfo.files[i];
        if(fname.substr(fname.size() - 4, 4) == ".vdb"){
            loadFile(fname); // TODO not supported on linux sadly
        } else {
            std::cout << "Unsupported file type for " << fname << std::endl;
        }
    }
}

//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------

void ofApp::loadFile(const string &fname)
{
    std::cout << "Loading grids from " << fname << std::endl;
    // load file
    openvdb::io::File file(fname);
    file.open();
    for (openvdb::io::File::NameIterator nameIter = file.beginName();
        nameIter != file.endName(); ++nameIter){
        // read grid
        ovdbGridPtr newGrid = openvdb::gridPtrCast<ovdbGrid>(file.readGrid(nameIter.gridName()));
        grid->merge(*newGrid);
        grid->pruneGrid();
    }
    file.close();
}

float ofApp::getCellSize() const
{
    return 1.0f / float(1 << resolveLevel);
}

void ofApp::drawUI()
{
    ofDisableDepthTest();
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
        // draw resolution slider
        {
            ofRectangle rect = resolveSlider();
            bool resolveHover = rect.inside(mouseX, mouseY) || dragLevel;
            ofFill();
            // outside
            if(resolveHover)
                ofSetColor(0, 0, 0, 200);
            else
                ofSetColor(0, 0, 0, 100);
            ofRect(rect);
            // inside
            ofSetColor(0, 100, 0, resolveHover ? 255 : 100);
            rect.scaleFromCenter(0.7f, 0.99f);
            rect.scaleHeight(0.1f + 0.9f * float(resolveLevel) / MAX_RESOLUTION);
            ofRect(rect);
            // count
            ofSetColor(255, 255, 255);
            char countChar = '0' + resolveLevel;
            char count[2] = { countChar, '\0' };
            ofDrawBitmapString(count, rect.x + rect.width / 3, rect.y + 15);
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
    ofEnableDepthTest();
}

void ofApp::drawGrid()
{
    cam.begin();

    // 1 = draw ground
    ofFill();
    ofSetColor(100, 100, 100, 100);
    ofDrawPlane(20.0f, 20.0f);

    // 2 = draw selection plane
    ofPushMatrix();
    {
        ofTranslate(0.0f, 0.0f, currentZ + 1e-5f);
        ofSetColor(0, 100, 255, 255);
        float ticks = 10.0f * float(1 << resolveLevel);
        ofDrawGrid(10.0f, ticks, false, false, false, true);
    }
    ofPopMatrix();

    // 3 = draw z-axis if in z mode
    if(ofGetKeyPressed('z')){
        ofNoFill();
        ofSetColor(0, 0, 255, 255);
        ofDrawBox(0.0f, 0.0f, 5.0f, 20.0f, 20.0f, 10.0f);
    }

    // 4 = display selected cell
    if(currentTool != Move){
        // show selection on grid
        ofSetColor(255, 255, 255, 100);

        ofVec3f pos = mouseOnPlane();
        float cellSize = getCellSize();
        float cellRadius = cellSize * 0.5f;
        float voxX = std::floor(pos.x / cellSize) * cellSize;
        float voxY = std::floor(pos.y / cellSize) * cellSize;
        ofFill();
        ofDrawPlane(voxX + cellRadius, voxY + cellRadius, pos.z, cellSize, cellSize);
        // std::cout << "@" << voxX << ", " << voxY << std::endl;
    }

    // 5 = display voxels
    // std::cout << "Voxels:\n";
    ofPushMatrix();
    float scale = 1.0f / float(1 << 5);
    ofScale(scale, scale, scale);
    for (typename ovdbGrid::ValueOnIter iter = grid->beginValueOn(); iter; ++iter) {
        ovdbCoordBBox bbox = iter.getBoundingBox();
        const ovdbCoord &min = bbox.min(), &max = bbox.max();
        ofFill();
        ofSetColor(255, 255, 255);
        ofDrawBox(min[0], min[1], min[2],
                  max[0] - min[0], max[1] - min[1], max[2] - min[2]);
        // std::cout << ". " <<  << std::endl;
    }
    ofPopMatrix();

    cam.end();
}

void ofApp::selectTool(Tool tool)
{
    if(tool == Unknown) return;
    // change state
    ui[currentTool].state = false; // toggle current off
    ui[tool].state = true;
    // update current tool
    currentTool = tool;
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
}
