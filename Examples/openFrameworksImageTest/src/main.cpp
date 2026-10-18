// ==========================================================
// openFrameworks Image Test Example
//
// This file is part of FreeImage 3
//
// Copyright (C) 2003-2026 The FreeImage Project
//
// Use at own risk!
// ==========================================================
//
//  Standard openFrameworks entry point. See ofApp.cpp for the
//  actual FreeImage test suite this example runs.
//
// ==========================================================

#include "ofMain.h"
#include "ofApp.h"

int main() {
	ofGLFWWindowSettings settings;
	settings.setSize(1024, 768);
	settings.windowMode = OF_WINDOW;

	auto window = ofCreateWindow(settings);

	ofRunApp(window, std::make_shared<ofApp>());
	return ofRunMainLoop();
}
