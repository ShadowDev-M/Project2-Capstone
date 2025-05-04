#pragma once
#include "tinyxml2.h"
#include <iostream>
using namespace tinyxml2;

class XMLtest {
public:
	void readDoc() {
		XMLDocument doc;

		doc.LoadFile("test.xml");

		bool status = doc.Error();

		//printf("%d", status);

		if (status != XML_SUCCESS) return;

		//doc.LoadFile("DecisionTree");

		XMLElement* rootData = doc.FirstChildElement();

		for (XMLElement* e = rootData->FirstChildElement(); e != nullptr; e = e->NextSiblingElement()) {
			std::cout << "Element [" << e->Value() << "]: ";
		}
		std::cout << std::endl;
	}
};