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
//  Loads every image FreeImage's own TestAPI suite ships as a
//  checked-in fixture (bin/data/ here is a copy of TestAPI's
//  tracked sample files) through openFrameworks' ofImage - which
//  is backed by this repo's FreeImage build when set up per the
//  README in this folder - and reports pass/fail per format both
//  on screen and in the console log.
//
// ==========================================================

#pragma once

#include "ofMain.h"

struct ImageTestResult {
	std::string filename;
	bool loaded;
	int width;
	int height;
};

class ofApp : public ofBaseApp {

public:
	void setup() override;
	void draw() override;
	void keyPressed(int key) override;

private:
	void runTests();

	std::vector<ofImage> images;
	std::vector<ImageTestResult> results;
};
