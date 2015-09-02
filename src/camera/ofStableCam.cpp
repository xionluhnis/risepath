#include "ofStableCam.h"
#include "ofMath.h"
#include "ofUtils.h"

// #include <iostream>

// when an ofStableCam is moving due to momentum, this keeps it
// from moving forever by assuming small values are zero.
float minDifference = 0.1e-5f;

// this is the default on windows os
unsigned long doubleclickTime = 200;

//----------------------------------------
ofStableCam::ofStableCam() : up(0, 0, 1){
	lastTap	= 0;
	lastDistance = 0;
	drag = 0.9f;
	sensitivityRot = 1.0f;//when 1 moving the mouse from one side to the other of the arcball (min(viewport.width, viewport.height)) will rotate 180degrees. when .5, 90 degrees.
	sensitivityXY = .5f;
	sensitivityZ= .7f;

	bDistanceSet = false;
	bMouseInputEnabled = false;
	bDoRotate = false;
	bApplyInertia =false;
	bDoTranslate = false;
	bInsideArcball = true;
	bValidClick = false;
	bEnableMouseMiddleButton = true;
	bAutoDistance = true;
	doTranslationKey = 'm';

	reset();
	enableMouseInput();

}

//----------------------------------------
ofStableCam::~ofStableCam(){
	disableMouseInput();
}
//----------------------------------------
void ofStableCam::update(ofEventArgs & args){
    if(!bDistanceSet && bAutoDistance){
        setDistance(getImagePlaneDistance(viewport), true);
    }
    if(bMouseInputEnabled){

		rotationFactor = sensitivityRot * 180 / min(viewport.width, viewport.height);
		if (bMouseInputEnabled) {
			updateMouse();
		}

		if (bDoRotate) {
			updateRotation();
		}else if (bDoTranslate) {
			updateTranslation();
		}
	}
}
//----------------------------------------
void ofStableCam::begin(ofRectangle viewport){
  fixUp();
	this->viewport = viewport;
	ofCamera::begin(viewport);
}

//----------------------------------------
void ofStableCam::reset(){
	target.resetTransform();

  /*
	target.setPosition(0,0, 0);
	lookAt(target);

	resetTransform();
	setPosition(0, 0, lastDistance);
  */

	xRot = 0;
	yRot = 0;
	zRot = 0;

	moveX = 0;
	moveY = 0;
	moveZ = 0;

	enableMouseMiddleButton();
  setDistance(13.0f);
  setNearClip(0.1f);
  setUp(ofVec3f(0.0f, 0.0f, 1.0f));
  setPosition(0, -10.0f, 3.0f);
  setTarget(ofVec3f(0.0f, 0.0f, 0.0f));
}
/**
 * Fix the camera to have ${up} as up vector all the time
 */
void ofStableCam::fixUp(){
  // do not fix while translating as there is no rotation change => no change for up
    // if(ofGetMousePressed(OF_MOUSE_BUTTON_MIDDLE)) return;
    // reset up direction unless we're perpendicular to it
    float angleZUp = getUpDir().dot(up);
    if(angleZUp > 1e-4f){
        ofVec3f camPos = getPosition();
        // ofVec3f trgPos = getTarget().getPosition();
        lookAt(getTarget(), up);
        setPosition(camPos);
        //setTarget(trgPos);
    }
}
//----------------------------------------
void ofStableCam::setTarget(const ofVec3f& targetPoint){
	target.setPosition(targetPoint);
	lookAt(target);
}
//----------------------------------------
void ofStableCam::setTarget(ofNode& targetNode){
	target = targetNode;
	lookAt(target);
}
//----------------------------------------
ofNode& ofStableCam::getTarget(){
	return target;
}
//----------------------------------------
void ofStableCam::setUp(const ofVec3f& u){
	up = u;
}
//----------------------------------------
const ofVec3f& ofStableCam::getUp() const{
	return up;
}
//----------------------------------------
void ofStableCam::setDistance(float distance){
	setDistance(distance, true);
}
//----------------------------------------
void ofStableCam::setDistance(float distance, bool save){//should this be the distance from the camera to the target?
	if (distance > 0.0f){
		if(save){
			this->lastDistance = distance;
		}
		setPosition(target.getPosition() + (distance * getZAxis()));
		bDistanceSet = true;
	}
}
//----------------------------------------
float ofStableCam::getDistance() const {
	return target.getPosition().distance(getPosition());
}
//----------------------------------------
void ofStableCam::setAutoDistance(bool bAutoDistance){
    this->bAutoDistance = bAutoDistance;
    if (bAutoDistance) {
        bDistanceSet = false;
    }
}
//----------------------------------------
void ofStableCam::setDrag(float drag){
	this->drag = drag;
}
//----------------------------------------
float ofStableCam::getDrag() const {
	return drag;
}
//----------------------------------------
void ofStableCam::setTranslationKey(char key){
	doTranslationKey = key;
}
//----------------------------------------
char ofStableCam::getTranslationKey(){
	return doTranslationKey;
}
//----------------------------------------
void ofStableCam::enableMouseInput(){
	if(!bMouseInputEnabled){
		bMouseInputEnabled = true;
	//	ofRegisterMouseEvents(this);
		ofAddListener(ofEvents().update , this, &ofStableCam::update);
	}
}
//----------------------------------------
void ofStableCam::disableMouseInput(){
	if(bMouseInputEnabled){
		bMouseInputEnabled = false;
		//ofUnregisterMouseEvents(this);
		ofRemoveListener(ofEvents().update, this, &ofStableCam::update);
	}
}
//----------------------------------------
bool ofStableCam::getMouseInputEnabled(){
	return bMouseInputEnabled;
}
//----------------------------------------
void ofStableCam::enableMouseMiddleButton(){
	bEnableMouseMiddleButton = true;
}
//----------------------------------------
void ofStableCam::disableMouseMiddleButton(){
	bEnableMouseMiddleButton = false;
}
//----------------------------------------
bool ofStableCam::getMouseMiddleButtonEnabled(){
	return bEnableMouseMiddleButton;
}
//----------------------------------------
void ofStableCam::updateTranslation(){
	if (bApplyInertia) {
		moveX *= drag;
		moveY *= drag;
		moveZ *= drag;
		if (ABS(moveX) <= minDifference && ABS(moveY) <= minDifference && ABS(moveZ) <= minDifference) {
			bApplyInertia = false;
			bDoTranslate = false;
		}
	}
	ofVec3f deltaXY = getXAxis() * moveX + getYAxis() * moveY;
	ofVec3f deltaZ =  getZAxis() * moveZ;
  // stable movement => move both position and target
	target.move(deltaXY);
	move(deltaXY + deltaZ);
}
//----------------------------------------
void ofStableCam::updateRotation(){
	if (bApplyInertia) {
		xRot *=drag;
		yRot *=drag;
		zRot *=drag;

		if (ABS(xRot) <= minDifference && ABS(yRot) <= minDifference && ABS(zRot) <= minDifference) {
			bApplyInertia = false;
			bDoRotate = false;
		}
	}
	curRot = ofQuaternion(xRot, ofCamera::getXAxis(), yRot, ofCamera::getYAxis(), zRot, ofCamera::getZAxis());
	setPosition((ofCamera::getGlobalPosition()-target.getGlobalPosition())*curRot +target.getGlobalPosition());
	rotate(curRot);
}
//----------------------------------------
void ofStableCam::updateMouse(){
	mouse = ofVec2f(ofGetMouseX(), ofGetMouseY());
	if(viewport.inside(mouse.x, mouse.y) && !bValidClick && ofGetMousePressed()){
		unsigned long curTap = ofGetElapsedTimeMillis();
		if(lastTap != 0 && curTap - lastTap < doubleclickTime){
			reset();
		}

		if ((bEnableMouseMiddleButton && ofGetMousePressed(OF_MOUSE_BUTTON_MIDDLE)) || ofGetKeyPressed(doTranslationKey)  || ofGetMousePressed(OF_MOUSE_BUTTON_RIGHT)){
			bDoTranslate = true;
			bDoRotate = false;
			bApplyInertia = false;
		}else if (ofGetMousePressed(OF_MOUSE_BUTTON_LEFT)) {
			bDoTranslate = false;
			bDoRotate = true;
			bApplyInertia = false;
			if(ofVec2f(mouse.x - viewport.x - (viewport.width/2), mouse.y - viewport.y - (viewport.height/2)).length() < min(viewport.width/2, viewport.height/2)){
				bInsideArcball = true;
			}else {
				bInsideArcball = false;
			}
		}
		lastTap = curTap;
		//lastMouse = ofVec2f(ofGetPreviousMouseX(),ofGetPreviousMouseY()); //this was causing the camera to have a tiny "random" rotation when clicked.
		lastMouse = mouse;
		bValidClick = true;
		bApplyInertia = false;
	}

	if (bValidClick) {
		if (!ofGetMousePressed()) {
			bApplyInertia = true;
			bValidClick = false;
		}else {
			int vFlip;
			if(isVFlipped()){
				vFlip = -1;
			}else{
				vFlip =  1;
			}

			mouseVel = mouse  - lastMouse;

			if (bDoTranslate) {
				moveX = 0;
				moveY = 0;
				moveZ = 0;
				if (ofGetMousePressed(OF_MOUSE_BUTTON_RIGHT)) {
					moveZ = mouseVel.y * sensitivityZ * (getDistance() + FLT_EPSILON)/ viewport.height;
				}else {
					moveX = -mouseVel.x * sensitivityXY * (getDistance() + FLT_EPSILON)/viewport.width;
					moveY = vFlip * mouseVel.y * sensitivityXY * (getDistance() + FLT_EPSILON)/viewport.height;
				}
			}else {
				xRot = 0;
				yRot = 0;
				zRot = 0;
				if (bInsideArcball) {
					xRot = vFlip * -mouseVel.y * rotationFactor;
					yRot = -mouseVel.x * rotationFactor;
				}else {
					ofVec2f center(viewport.width/2, viewport.height/2);
					zRot = - vFlip * ofVec2f(mouse.x - viewport.x - center.x, mouse.y - viewport.y - center.y).angle(lastMouse - ofVec2f(viewport.x, viewport.y) - center);
				}
			}
			lastMouse = mouse;
		}
	}
}
