#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <algorithm>

#include "helper.h"

#include "node.h"

int screen_width, screen_height;

double scrollWheel = 0.0;

unsigned int program;

bool resize = true;

int txWidth = 100*5;
int txHeight = 100;

int keyStates[GLFW_KEY_LAST]{0};

void framebufferSizeCallback(GLFWwindow* window, int width, int height){
	screen_width = width;
	screen_height = height;

	resize = true;
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset){
	scrollWheel += yOffset;
}
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods){
	if(action == GLFW_RELEASE) keyStates[key] = -1;
	else if(action == GLFW_PRESS) keyStates[key] = 1;
}
void mouseButtonCallback(GLFWwindow *window, int key, int action, int mods){
	keyCallback(window, key, 0, action, 0);
}

void countShapes(std::vector<Node*>& nodes, int& rectangleCount, int& lineCount){

	rectangleCount = 0;
	lineCount = 0;

	for(int i = 0; i < (int)nodes.size(); i++){
		rectangleCount += nodes.at(i)->getRectCount();
		lineCount += nodes.at(i)->getLineCount();
	}
}
void loadShapes(std::vector<Node*>& nodes, int& rectangleCount, int& lineCount, int& vertexCount, vertex_rectangle* rectangles, line* lines){
	int rc = 0;
	int lc = 0;
	for(int i = 0; i < (int)nodes.size(); i++){
		for(int j = 0; j < nodes.at(i)->getRectCount(); j++){
			*(rectangles+rc++) = nodes.at(i)->getRect(j);
		}
		for(int j = 0; j < nodes.at(i)->getLineCount(); j++){
			*(lines+lc++) = nodes.at(i)->getLine(j);
		}
	}

	vertexCount = rectangleCount*4 + lineCount*2;
}

void convertShapes(int& rectangleCount, int& lineCount, int& vertexCount, vertex_rectangle* rectangles, line* lines, vertex* vertices){

	for(int i = 0; i < rectangleCount; i++){
		vertex_rectangle r = *(rectangles+i);
		float txx = r.texture / (txWidth/(float)txHeight);
		float one = (txHeight/(float)txWidth);
		*(vertices+i*4+0) = vertex{		 r.position,													r.color, r.texture == 0 ? vec2f{0.0f} : vec2f{txx, 0.0f}};
		*(vertices+i*4+1) = vertex{vec2f{r.position.x + r.dimensions.x, r.position.y},					r.color, r.texture == 0 ? vec2f{0.0f} : vec2f{txx+one, 0.0f}};
		*(vertices+i*4+2) = vertex{vec2f{r.position.x + r.dimensions.x, r.position.y + r.dimensions.y}, r.color, r.texture == 0 ? vec2f{0.0f} : vec2f{txx+one, 1.0f}};
		*(vertices+i*4+3) = vertex{vec2f{r.position.x,					r.position.y + r.dimensions.y}, r.color, r.texture == 0 ? vec2f{0.0f} : vec2f{txx, 1.0f}};
	}
	for(int i = 0; i < lineCount; i++){
		line l = *(lines+i);
		*(vertices+rectangleCount*4+i*2+0) = vertex{l.position1, l.color, vec2f{0.0f}};
		*(vertices+rectangleCount*4+i*2+1) = vertex{l.position2, l.color, vec2f{0.0f}};
	}
}

int main(){

	GLFWwindow* window;

	if(!glfwInit()){
		std::cerr << "GLFW could not be initiated." << std::endl;
		return -1;
	}

	window = glfwCreateWindow(800, 600, "Connect", NULL, NULL);
	if(!window){
		std::cerr << "Could not create window." << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwGetFramebufferSize(window, &screen_width, &screen_height);

	glfwSetScrollCallback(window, scrollCallback);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	//glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
	//glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

	GLFWimage windowIcon;
	windowIcon.width = 256;
	windowIcon.height = 256;
	windowIcon.pixels = readImage("res/icons/icon.data", windowIcon.width, windowIcon.height);

	glfwSetWindowIcon(window, 1, &windowIcon);

	if(glewInit() != GLEW_OK){
		std::cerr << "GLEW could not be initiated." << std::endl;
		glfwTerminate();
		return -1;
	}

	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glfwSwapInterval(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(!Node::readSave("nodesave.data")){
		std::cout << "Did not read." << std::endl;

		ButtonNode* b1 = new ButtonNode();
		Node::addToWorld((Node*)b1);
	}

	std::vector<Node*> guiNodes;
	{
		Node* guin1 = (Node*)new ButtonNode();
		Node* guin2 = new Node(2, 1, nodeAndCondition, 1);
		Node* guin3 = new Node(1, 1, nodeNotCondition, 2);
		Node* guin4 = new Node(2, 1, nodeXorCondition, 3);
		Node* guin5 = new Node(1, 1, nodeOrCondition, 4);
		Node* guin6 = new Node(16, 9, nodeAdder, 0);

		guin1->pos.x = Node::WIDTH*(0*1.5f + 0.5f);
		guin2->pos.x = Node::WIDTH*(1*1.5f + 0.5f);
		guin3->pos.x = Node::WIDTH*(2*1.5f + 0.5f);
		guin4->pos.x = Node::WIDTH*(3*1.5f + 0.5f);
		guin5->pos.x = Node::WIDTH*(4*1.5f + 0.5f);
		guin6->pos.x = Node::WIDTH*(5*1.5f + 0.5f);

		guin1->pos.y = Node::WIDTH/2.0f;
		guin2->pos.y = Node::WIDTH/2.0f;
		guin3->pos.y = Node::WIDTH/2.0f;
		guin4->pos.y = Node::WIDTH/2.0f;
		guin5->pos.y = Node::WIDTH/2.0f;
		guin6->pos.y = Node::WIDTH/2.0f;

		guiNodes.push_back(guin1);
		guiNodes.push_back(guin2);
		guiNodes.push_back(guin3);
		guiNodes.push_back(guin4);
		guiNodes.push_back(guin5);
		//guiNodes.push_back(guin6);
	}



	int rectangleCount = 0;
	int lineCount = 0;
	int vertexCount = 0;

	int maxRectangleCount = 1024;
	int maxLineCount = 1024;
	int maxVertexCount = 1024;

	vertex_rectangle* rectangles = new vertex_rectangle[maxRectangleCount];
	line* lines = new line[maxLineCount];
	vertex* vertices = new vertex[maxVertexCount];

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, maxVertexCount * sizeof(struct vertex), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (const void*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (const void*)sizeof(struct vec2f));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (const void*)(sizeof(struct vec2f)+sizeof(struct vec4f)));

	int indexCount = maxVertexCount/4 * 6;
	unsigned int* indices = new unsigned int[indexCount];

	for(int i = 0; i < maxVertexCount/4; i++){
		*(indices+i*6+0) = i*4+0;
		*(indices+i*6+1) = i*4+1;
		*(indices+i*6+2) = i*4+2;
		*(indices+i*6+3) = i*4+2;
		*(indices+i*6+4) = i*4+3;
		*(indices+i*6+5) = i*4+0;
	}

	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	program = createShader("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");
	if(program == 0){
		std::cerr << "Could not create shader!" << std::endl;
		glfwTerminate();
		return -1;
	}
	glUseProgram(program);

	unsigned char* imageData = readImage("res/textures/icons2.data", txWidth, txHeight);
	if(imageData == 0){
		std::cerr << "Could not read image." << std::endl;
		return -1;
	}
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, txWidth, txHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);
	delete[] imageData;
	glUniform1i(glGetUniformLocation(program, "u_texture"), 0);

	vec2f translation = {0.0f, 0.0f};
	float scale = 0.1f;

	vec2f oldMouse;

	vec2f nodeMoveStart;
	Node* nodeMove = nullptr;

	float scaleR = 0.08f;

	Node* selectedOut = nullptr;
	Node* selectedIn = nullptr;
	int selectedInConnector = -1;
	int selectedOutConnector = -1;

	line dragLine = line{vec2f{0.0f, 0.0f}, vec2f{0.0f, 0.0f}, vec4f{0.0f, 0.0f, 1.0f, 1.0f}};
	bool shallDrawDragLine = false;

	rectangle selection = rectangle{vec2f{0.0f, 0.0f}, vec2f{0.0f, 0.0f}};
	vec2f selectionStart;
	vec2f selectionEnd;
	int selectCount = 0;

	vertex guiRectVertices[4];
	rectangle guiRect;
	int guiHeight = 250;

	vec2f guiTranslation = vec2f{0.0f, 0.0f};
	float guiScale = 100.0f / (float)screen_width;

	bool redraw = true;

	long long signed int frames = 0;
	double startTime = glfwGetTime();
	double loopTime = 0.0;
	double loopStartTime;

	while(!glfwWindowShouldClose(window)){
		loopStartTime = glfwGetTime();

		if(resize){
			glViewport(0,0, screen_width, screen_height);

			if(!setProjectionMatrix(program, -1.0f, 1.0f, 1.0f * (screen_height/(float)screen_width), -1.0f * (screen_height/(float)screen_width))){
				std::cerr << "Could not set projection matrix!" << std::endl;
				goto program_exit;
			}

			{
				vec2f guiDims = vec2f{
					2.0f, guiHeight / (float)screen_width
				};
				vec2f guiPos = vec2f{
					0.0f, 0.0f
				};
				guiRect = rectangle{
					guiPos, guiDims
				};

				vec4f guiColor = vec4f{0.3f, 0.3f, 0.3f, 1.0f};
				guiRectVertices[0] = vertex{guiRect.position, guiColor, vec2f{0.0f}};
				guiRectVertices[1] = vertex{guiRect.position + vec2f{guiRect.dimensions.x, 0.0f}, guiColor, vec2f{0.0f}};
				guiRectVertices[2] = vertex{guiRect.position + guiRect.dimensions, guiColor, vec2f{0.0f}};
				guiRectVertices[3] = vertex{guiRect.position + vec2f{0.0f, guiRect.dimensions.y}, guiColor, vec2f{0.0f}};

				guiScale = 100.0f / (float)screen_width;
				guiTranslation = vec2f{-1.0f/guiScale, -(screen_height/(float)screen_width)/guiScale};
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		double mx_get, my_get;
		glfwGetCursorPos(window, &mx_get, &my_get);

		vec2f mouse; // Mouse position in screen space (in range (-1 - 1, -1 - 1))
		mouse.x = 2.0f*mx_get/float(screen_width) - 1.0f;
		mouse.y = (2.0f*(screen_height - my_get - 1)/float(screen_height) - 1.0f)*(screen_height/(float)screen_width);

		//if(glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS && oldKeyF1State == GLFW_RELEASE){
		if(keyStates[GLFW_KEY_F1] == 1){
			if(!Node::writeSave("nodesave.data")){
				std::cout << "Did not save." << std::endl;
			}
			else{
				std::cout << "Saved!" << std::endl;
			}
		}

		//if(mouse.y < screenToWorld(guiRect.dimensions, guiTranslation, guiScale).y){
		if(screen_height-my_get-1 < guiHeight/2){

			vec2f worldMouse = screenToWorld(mouse, guiTranslation, guiScale);

			for(int i = 0; i < (int)guiNodes.size(); i++){
				if(overlap(worldMouse, guiNodes.at(i)->getRect())){
					guiNodes.at(i)->highlight = true;
					//if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && oldMouseLState == GLFW_RELEASE){
					if(keyStates[GLFW_MOUSE_BUTTON_LEFT] == 1){
						Node* newNode = guiNodes.at(i)->createNew();
						Node::addToWorld(newNode);
					}
				}
				else{
					guiNodes.at(i)->highlight = false;
				}
			}
		}
		else{

			if(keyStates[GLFW_MOUSE_BUTTON_MIDDLE] > 0){
				translation.x += (mouse.x - oldMouse.x)/scale;
				translation.y += (mouse.y - oldMouse.y)/scale;
			}

			if(scrollWheel != 0.0){
				float scaleFactor = (1+scaleR*scrollWheel);

				scale *= scaleFactor;

				translation.x += (mouse.x - mouse.x*scaleFactor)/scale;
				translation.y += (mouse.y - mouse.y*scaleFactor)/scale;
			}

			vec2f worldMouse = screenToWorld(mouse, translation, scale);

			if(keyStates[GLFW_MOUSE_BUTTON_LEFT] > 0 && selectedIn == nullptr && selectedOut == nullptr){ // Drag a selection
				if(keyStates[GLFW_MOUSE_BUTTON_LEFT] == 1){
					selectionStart = worldMouse;
					selectionEnd = worldMouse;
					selection = toRect(selectionStart, selectionEnd);
				}
				else if(keyStates[GLFW_MOUSE_BUTTON_LEFT] > 1){
					selectionEnd = worldMouse;
					selection = toRect(selectionStart, selectionEnd);
				}
				for(int i = 0; i < (int)Node::getAllNodes().size(); i++){ // Highlight nodes in selection
					Node* n = Node::getAllNodes().at(i);
					//if(selectionStart == selectionEnd &&)
					//if((selection.dimensions.x != 0 && selection.dimensions.x != 0) && overlap(selection, n->getRect())){
					//if(overlap(selection, n->getRect()) || (keyStates[GLFW_MOUSE_BUTTON_LEFT] == -1 && overlap(worldMouse, n->getRect()))){
					if(overlap(selection, n->getRect())){
						n->highlight = true;
					}
					else{
						n->highlight = false;
					}
				}
			}
			else{
				selection = rectangle{0.0f, 0.0f, 0.0f, 0.0f};
				selectionStart = vec2f{0.0f, 0.0f};
				selectionEnd = selectionStart;
			}
			selectCount = 0;
			for(int i = 0; i < (int)Node::getAllNodes().size(); i++){ // Highlight nodes in selection
				Node* n = Node::getAllNodes().at(i);
				//if(overlap(worldMouse, n->getRect()) ||
				//	((selection.dimensions.x != 0 && selection.dimensions.x != 0) && overlap(selection, n->getRect()))){
				if(n->highlight == true){

					if(keyStates[GLFW_KEY_DELETE] == 1 || keyStates[GLFW_KEY_BACKSPACE] == 1){
						delete n;
						i--;
						continue;
					}
					if(keyStates[GLFW_KEY_L] == 1){
						n->resize(n->getInputCount()+1, n->getOutputCount());
					}
					if(keyStates[GLFW_KEY_K] == 1){
						n->resize(n->getInputCount()-1, n->getOutputCount());
					}
					selectCount++;
				}
			}

			if(keyStates[GLFW_KEY_D] == 1){ // Duplicate all highlighted nodes.
				std::vector<Node*> copyFrom;
				std::vector<Node*> newNodes;
				for(int i = 0; i < (int)Node::getAllNodes().size(); i++){
					Node* n = Node::getAllNodes().at(i);
					if(n->highlight){
						copyFrom.push_back(n);
						Node* newNode = n->createNew();
						newNode->pos = n->pos + vec2f{Node::WIDTH, 0.0f};
						newNodes.push_back(newNode);
					}
				}
				for(int i = 0; i < (int)copyFrom.size(); i++){
					Node* f = copyFrom.at(i);
					Node* t = newNodes.at(i);

					for(int j = 0; j < f->getInputCount(); j++){
						for(int k = 0; k < f->getConnectionCount(j); k++){
							auto indexIt = std::find(copyFrom.begin(), copyFrom.end(), f->getConnection(j, k).first);
							if(indexIt != copyFrom.end()){
								int index = indexIt - copyFrom.begin();
								t->addInput(j, newNodes.at(index), f->getConnection(j, k).second);
							}
						}
					}
				}
				for(int i = 0; i < (int)newNodes.size(); i++){
					Node* f = copyFrom.at(i);
					Node* t = newNodes.at(i);

					f->highlight = false;
					t->highlight = true;
					Node::addToWorld(t);
				}
			}

			if(keyStates[GLFW_MOUSE_BUTTON_RIGHT] > 0){ // Right click to move nodes.
				//if(nodeMove == nullptr && keyStates[GLFW_MOUSE_BUTTON_RIGHT] == -1){ // Find node to move.
				if(keyStates[GLFW_MOUSE_BUTTON_RIGHT] == 1){
					if(nodeMove == nullptr){ // Find node to move.
						for(int i = 0; i < (int)Node::getAllNodes().size(); i++){
							Node* n = Node::getAllNodes().at(i);
							if(overlap(worldMouse, rectangle{n->getPos(), n->getDimensions()})){
								nodeMove = n;
							}
						}
					}
					nodeMoveStart = worldMouse; // Unused
				}

				if(nodeMove != nullptr){ // Move the node nodeMove.
					nodeMove->pos += worldMouse - screenToWorld(oldMouse, translation, scale);
				}
				else{
					for(int i = 0; i < (int)Node::getAllNodes().size(); i++){
						Node* n = Node::getAllNodes().at(i);
						if(n->highlight) n->pos += worldMouse - screenToWorld(oldMouse, translation, scale);
					}
				}

			}
			else if(keyStates[GLFW_MOUSE_BUTTON_RIGHT] == 0){
				nodeMove = nullptr;
			}

			if(keyStates[GLFW_MOUSE_BUTTON_LEFT] == 1){ // Left click to toggle a button
				for(int i = 0; i < (int)Node::getAllNodes().size(); i++){
					Node* n = Node::getAllNodes().at(i);
					if(overlap(worldMouse, n->getRect())){
						n->onClick(worldMouse - n->getPos());
					}
				}
			}

			if(keyStates[GLFW_MOUSE_BUTTON_LEFT] == 1 || keyStates[GLFW_MOUSE_BUTTON_LEFT] == -1){ // Drag between two connectors to connect them.
			//if(keyStates[GLFW_MOUSE_BUTTON_LEFT] != 0){ // Drag between two connectors to connect them.
				for(int i = 0; i < (int)Node::getAllNodes().size(); i++){
					Node* n = Node::getAllNodes().at(i);
					for(int j = 0; j < n->getInputCount(); j++){
						if(overlap(worldMouse, rectangle{n->getInputConnectorPos(j), vec2f{Node::CONNECTOR_SIZE, Node::CONNECTOR_SIZE}})){
							selectedIn = n;
							selectedInConnector = j;
							dragLine.position1 = selectedIn->getInputConnectorPos(selectedInConnector) + vec2f{Node::CONNECTOR_SIZE/2, Node::CONNECTOR_SIZE/2};
							shallDrawDragLine = true;
							break;
						}
					}
					for(int j = 0; j < n->getOutputCount(); j++){
						if(overlap(worldMouse, rectangle{n->getOutputConnectorPos(j), vec2f{Node::CONNECTOR_SIZE, Node::CONNECTOR_SIZE}})){
							selectedOut = n;
							selectedOutConnector = j;
							dragLine.position1 = selectedOut->getOutputConnectorPos(selectedOutConnector) + vec2f{Node::CONNECTOR_SIZE/2, Node::CONNECTOR_SIZE/2};
							shallDrawDragLine = true;
							break;
						}
					}
				}
			}

			if(selectedIn != nullptr && selectedOut != nullptr){ // Connect the nodes.
				if(selectedIn->hasInput(selectedInConnector, selectedOut, selectedOutConnector)){
					selectedIn->removeInput(selectedInConnector, selectedOut, selectedOutConnector);
				}
				else{
					selectedIn->addInput(selectedInConnector, selectedOut, selectedOutConnector);
				}
				selectedIn = nullptr;
				selectedOut = nullptr;
				selectedInConnector = -1;
				selectedOutConnector = -1;
				shallDrawDragLine = false;
				redraw = true;
			}

			if(selectedIn != nullptr || selectedOut != nullptr){ // Draw line between mouse and connector while you are draging.
				dragLine.position2 = worldMouse;
			}

			if(keyStates[GLFW_MOUSE_BUTTON_LEFT] == 0){ // Stop connecting two nodes if you release the mouse key.
				selectedIn = nullptr;
				selectedOut = nullptr;
				selectedInConnector = -1;
				selectedOutConnector = -1;
				shallDrawDragLine = false;
			}

			/*if(keyStates[GLFW_MOUSE_BUTTON_RIGHT] != 0){ // Right click to move nodes.
				if(nodeMove == nullptr && keyStates[GLFW_MOUSE_BUTTON_RIGHT] == -1){ // Find node to move.
					for(int i = 0; i < (int)Node::getAllNodes().size(); i++){
						Node* n = Node::getAllNodes().at(i);
						//if(overlap(n->getPos(), n->getDimensions(), worldMouse)){
						if(overlap(worldMouse, rectangle{n->getPos(), n->getDimensions()})){
							nodeMove = n;
							//nodeMoveDelta = vec2f{n->getPos().x - worldMouse.x, n->getPos().y - worldMouse.y};
							nodeMoveDelta = n->getPos() - worldMouse;
						}
					}
				}
				else if(nodeMove != nullptr){ // Move the node nodeMove.
					//float tx = round(worldMouse.x + nodeMoveDelta.x);
					//float ty = round(worldMouse.y + nodeMoveDelta.y);
					float tx = worldMouse.x + nodeMoveDelta.x;
					float ty = worldMouse.y + nodeMoveDelta.y;
					if(tx - round(tx) < 0.25f && tx - round(tx) > -0.25f) tx = round(tx);
					if(ty - round(ty) < 0.25f && ty - round(ty) > -0.25f) ty = round(ty);
					bool fail = false;
					for(auto n : Node::getAllNodes()){
						if(n == nodeMove) continue;
						//if(overlap(n->getPos(), n->getDimensions(), vec2f{tx, ty}, nodeMove->getDimensions())){
						if(overlap(rectangle{n->getPos(), n->getDimensions()}, rectangle{vec2f{tx, ty}, nodeMove->getDimensions()})){
							fail = true;
							break;
						}
					}
					if(!fail){
						nodeMove->getPos().x = tx;
						nodeMove->getPos().y = ty;
						redraw = true;
					}
				}

			}
			else if(keyStates[GLFW_MOUSE_BUTTON_RIGHT] == 0){
				nodeMove = nullptr;
			}*/

		}

		for(int i = 0; i < (int)Node::getAllNodes().size(); i++){ // Calculate...
			Node::getAllNodes().at(i)->update();
		}
		for(int i = 0; i < (int)Node::getAllNodes().size(); i++){ // ... and set outputs
			Node::getAllNodes().at(i)->setOut();
		}

		glUniform1f(glGetUniformLocation(program, "u_scale"), scale);
		glUniform2f(glGetUniformLocation(program, "u_translation"), translation.x, translation.y);

		redraw = true;
		if(redraw){
			countShapes(Node::getAllNodes(), rectangleCount, lineCount);

			// Hack in space for special shapes.
			if(shallDrawDragLine) lineCount++;
			if(selection.dimensions.x > 0.0f && selection.dimensions.y > 0.0f) rectangleCount++;

			if(rectangleCount > maxRectangleCount){
				std::cout << "Too many rectangles! " << "rc: " << rectangleCount << " / " << maxRectangleCount << std::endl;
				//maxRectangleCount += 1024;
				maxRectangleCount += 1024 * ((rectangleCount - maxRectangleCount) / 1024 + 1);
				delete[] rectangles;
				rectangles = new vertex_rectangle[maxRectangleCount];
			}
			if(lineCount > maxLineCount){
				std::cout << "Too many lines! " << "lc: " << lineCount << " / " << maxLineCount << std::endl;
				//maxLineCount += 1024;
				maxLineCount += 1024 * ((lineCount - maxLineCount) / 1024 + 1);
				delete[] lines;
				lines = new line[maxLineCount];
			}

			loadShapes(Node::getAllNodes(), rectangleCount, lineCount, vertexCount, rectangles, lines);

			// Hack in the special shapes.
			if(shallDrawDragLine){
				*(lines+lineCount-1) = dragLine;
			}
			if(selection.dimensions.x > 0.0f && selection.dimensions.y > 0.0f){
				*(rectangles+rectangleCount-1) = vertex_rectangle{selection.position, selection.dimensions, vec4f{1.0f, 0.8f, 0.0f, 0.5f}, 0};
			}

			if(vertexCount > maxVertexCount){
				std::cout << "Too many vertices! " << "vc: " << vertexCount << " / " << maxVertexCount << std::endl;
				//maxVertexCount += 1024;
				maxVertexCount += 1024 * ((vertexCount - maxVertexCount) / 1024 + 1);
				delete[] vertices;
				vertices = new vertex[maxVertexCount];
				glBufferData(GL_ARRAY_BUFFER, maxVertexCount * sizeof(struct vertex), nullptr, GL_DYNAMIC_DRAW);

				indexCount = maxVertexCount/4 * 6;
				delete[] indices;
				indices = new unsigned int[indexCount];
				for(int i = 0; i < maxVertexCount/4; i++){
					*(indices+i*6+0) = i*4+0;
					*(indices+i*6+1) = i*4+1;
					*(indices+i*6+2) = i*4+2;
					*(indices+i*6+3) = i*4+2;
					*(indices+i*6+4) = i*4+3;
					*(indices+i*6+5) = i*4+0;
				}
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
			}

			convertShapes(rectangleCount, lineCount, vertexCount, rectangles, lines, vertices);

			//glBufferSubData(GL_ARRAY_BUFFER, 0, maxVertexCount * sizeof(struct vertex), vertices);
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(struct vertex), vertices);
			redraw = false;
		}

		glDrawElements(GL_TRIANGLES, rectangleCount * 6, GL_UNSIGNED_INT, nullptr);
		glDrawArrays(GL_LINES, rectangleCount*4, lineCount*2);

		glUniform1f(glGetUniformLocation(program, "u_scale"), 1.0f);
		glUniform2f(glGetUniformLocation(program, "u_translation"), -1.0f, -(screen_height/(float)screen_width));
		glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(struct vertex), &guiRectVertices[0]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

		glUniform1f(glGetUniformLocation(program, "u_scale"), guiScale);
		glUniform2f(glGetUniformLocation(program, "u_translation"), guiTranslation.x, guiTranslation.y);

		countShapes(guiNodes, rectangleCount, lineCount);
		loadShapes(guiNodes, rectangleCount, lineCount, vertexCount, rectangles, lines);
		if(vertexCount > maxVertexCount){
			std::cout << "Out of memory 2!" << std::endl;
			goto program_exit;
		}
		convertShapes(rectangleCount, lineCount, vertexCount, rectangles, lines, vertices);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * sizeof(struct vertex), vertices);
		glDrawElements(GL_TRIANGLES, rectangleCount * 6, GL_UNSIGNED_INT, nullptr);

		oldMouse = mouse;

		for(int i = 0; i < GLFW_KEY_LAST; i++){
			//oldKeyStates[i] = keyStates[i];
			if(keyStates[i] == -1) keyStates[i] = 0;
			if(keyStates[i] > 0) keyStates[i]++;
		}

		scrollWheel = 0.0;

		loopTime += glfwGetTime()-loopStartTime;
		frames++;
		resize = false;

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	std::cout << "Average FPS: " << frames / (glfwGetTime()-startTime) << std::endl;
	std::cout << "Potential FPS: " << frames / loopTime << std::endl;

	program_exit:
	glDeleteProgram(program);

	glfwTerminate();
	return 0;
}
