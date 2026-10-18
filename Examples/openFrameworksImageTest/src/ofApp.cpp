// ==========================================================
// openFrameworks Image Test Example
//
// This file is part of FreeImage 3
//
// Copyright (C) 2003-2026 The FreeImage Project
//
// Use at own risk!
// ==========================================================

#include "ofApp.h"

namespace {
	// TestAPI's own checked-in source fixtures (as opposed to the many
	// other files under TestAPI/ that testMPage.cpp etc. generate at
	// test-run time) - copied into bin/data/ so this example works from
	// a plain `make run` without reaching back into the FreeImage tree.
	const std::vector<std::string> kTestImages = {
		"sample.png",
		"exif.jpg",
		"exif.jxr",
	};
}

void ofApp::setup() {
	ofSetWindowTitle("FreeImage + openFrameworks Image Test");
	ofBackground(30);
	ofSetFrameRate(30);

	runTests();
}

void ofApp::runTests() {
	images.clear();
	results.clear();

	for (const auto & filename : kTestImages) {
		ofImage img;
		bool loaded = img.load(filename);

		ImageTestResult result;
		result.filename = filename;
		result.loaded = loaded && img.getWidth() > 0 && img.getHeight() > 0;
		result.width = img.getWidth();
		result.height = img.getHeight();
		results.push_back(result);
		images.push_back(img);

		ofLogNotice("FreeImageTest") << filename << ": " << (result.loaded ? "PASS" : "FAIL")
			<< " (" << result.width << "x" << result.height << ")";
	}

	int failed = 0;
	for (const auto & r : results) {
		if (!r.loaded) failed++;
	}
	if (failed == 0) {
		ofLogNotice("FreeImageTest") << "All " << results.size() << " images loaded correctly.";
	} else {
		ofLogError("FreeImageTest") << failed << " of " << results.size() << " images failed to load.";
	}
}

void ofApp::draw() {
	int x = 20;
	int y = 20;
	const int thumbSize = 220;
	const int padding = 20;

	ofDrawBitmapStringHighlight("FreeImage + openFrameworks Image Test  (press 'r' to reload)", x, y);
	y += 30;

	for (size_t i = 0; i < images.size(); i++) {
		const ImageTestResult & result = results[i];

		if (result.loaded) {
			ofSetColor(255);
			images[i].draw(x, y, thumbSize, thumbSize);
		} else {
			ofSetColor(120, 30, 30);
			ofDrawRectangle(x, y, thumbSize, thumbSize);
		}

		ofSetColor(result.loaded ? ofColor(80, 220, 80) : ofColor(220, 80, 80));
		std::string status = result.loaded ? "PASS" : "FAIL";
		std::string label = result.filename + "  " + status + "  " +
			ofToString(result.width) + "x" + ofToString(result.height);
		ofDrawBitmapStringHighlight(label, x, y + thumbSize + 16);

		x += thumbSize + padding;
	}

	ofSetColor(255);
}

void ofApp::keyPressed(int key) {
	if (key == 'r') {
		runTests();
	}
}
