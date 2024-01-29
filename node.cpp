#include "node.h"

#include <algorithm>
#include <iostream>
#include <fstream>

#include <exception>

std::vector<Node*> Node::allNodes;

constexpr vec4f Node::onColor;
constexpr vec4f Node::offColor;
constexpr vec4f Node::bodyColor;
constexpr vec4f Node::bodyHighlightColor;

Node::Node(int inputCount, int outputCount, void (*condition)(Node* n, bool* values), int texture) : inputCount(inputCount), outputCount(outputCount), condition(condition), texture(texture) {
	inputs = new std::vector<std::pair<Node*, int>>[inputCount];

	currentValues = new bool[outputCount]{false};
	currentValues2 = new bool[outputCount]{false};

	pos = {0.0f, 0.0f};
	dimensions = {WIDTH, CONNECTOR_OFFSET + CONNECTOR_SIZE + (CONNECTOR_OFFSET+CONNECTOR_SIZE)*std::max(inputCount, outputCount)};

	type = 0;
}

Node::~Node(){
	Node::removeFromWorld(this);
	delete[] inputs;
	delete[] currentValues;
}

Node* Node::createNew(){
	if(type == 0){
		return new Node(inputCount, outputCount, condition, texture);
	}
	else if(type == 1){
		return (Node*) new ButtonNode();
	}
	std::cout << "BAD TYPE!!!" << std::endl;
	throw std::exception();
}

void Node::addToWorld(Node* node){
	allNodes.push_back(node);
}
void Node::removeFromWorld(Node* node){
	for(auto np : allNodes){
		np->removeInput(node);
	}
	auto nodePos = std::find(allNodes.begin(), allNodes.end(), node);
	if(nodePos != allNodes.end()) allNodes.erase(nodePos);
}

void Node::update(){
	if(condition != nullptr) condition(this, currentValues2);
}
void Node::setOut(){
	bool* tmp = currentValues;
	currentValues = currentValues2;
	currentValues2 = tmp;
}

int Node::addInput(int inConnector, Node* node, int outConnector){
	if(inConnector < 0 || inConnector >= inputCount) throw std::exception();
	for(int i = 0; i < (int)(inputs+inConnector)->size(); i++){
		if((inputs+inConnector)->at(i) == std::pair<Node*, int>(node, outConnector)) return i;
	}
	(inputs+inConnector)->push_back(std::pair<Node*, int>(node, outConnector));
	return (inputs+inConnector)->size(); // Why?
}
int Node::addInput(int inConnector, std::pair<Node*, int> connection){
	return addInput(inConnector, connection.first, connection.second);
}
bool Node::hasInput(int inConnector, Node* node, int outConnector){
	if(inConnector < 0 || inConnector >= inputCount) throw std::exception();
	for(int i = 0; i < (int)(inputs+inConnector)->size(); i++){
		if((inputs+inConnector)->at(i) == std::pair<Node*, int>(node, outConnector)) return true;
	}
	return false;
}
bool Node::hasInput(Node* node){
	for(int i = 0; i < inputCount; i++){
		for(int j = 0; j < (int)(inputs+i)->size(); j++){
			if((inputs+i)->at(i).first == node) return true;
		}
	}
	return false;
}
int Node::removeInput(int inConnector, Node* node, int outConnector){
	if(inConnector < 0 || inConnector >= inputCount) throw std::exception();
	for(int i = 0; i < (int)(inputs+inConnector)->size(); i++){
		if((inputs+inConnector)->at(i) == std::pair<Node*, int>(node, outConnector)){ // They are the same
			(inputs+inConnector)->erase((inputs+inConnector)->begin() + i);
			return 0;
		}
	}
	return 1; // Duno
}
int Node::removeInput(Node* node){
	for(int i = 0; i < inputCount; i++){
		for(int j = 0; j < (int)(inputs+i)->size(); j++){
			if((inputs+i)->at(j).first == node){
				(inputs+i)->erase((inputs+i)->begin() + j);
			}
		}
	}
	return 0; // Dunno
}
bool Node::getInputValue(int connector){
	for(int i = 0; i < getConnectionCount(connector); i++){ // Loop through all nodes at "connector". If any of them are true, return true.
		if((inputs+connector)->at(i).first->getValue((inputs+connector)->at(i).second)) return true;
	}
	return false;
}
bool Node::getValue(int connector){
	if(connector < 0 || connector >= outputCount) throw std::exception();
	return *(currentValues+connector);
}

int Node::getInputCount(){
	return inputCount;
}
int Node::getOutputCount(){
	return outputCount;
}
int Node::getType(){
	return type;
}

int Node::getConnectionCount(int connector){
	if(connector < 0 || connector >= inputCount) throw std::exception();
	return (inputs+connector)->size();
}
/*Node* Node::getNode(int connector, int count){
	if(connector < 0 || connector >= inputCount) return nullptr;
	if(count < 0 || count >= int((inputs+connector)->size())) return nullptr;
	return (inputs+connector)->at(count).first;
}*/
std::pair<Node*, int> Node::getConnection(int connector, int count){
	if(connector < 0 || connector >= inputCount) throw std::exception();
	if(count < 0 || count >= int((inputs+connector)->size())) throw std::exception();
	return (inputs+connector)->at(count);
}

void Node::resize(int newInputCount, int newOutputCount){
	if(newInputCount <= 0 || newOutputCount <= 0) return;
	std::vector<std::pair<Node*, int>>* newInputs = new std::vector<std::pair<Node*, int>>[newInputCount];
	for(int i = 0; i < inputCount; i++){
		if(i >= newInputCount) break;
		*(newInputs+i) = *(inputs+i);
	}
	delete[] inputs;
	inputs = newInputs;
	inputCount = newInputCount;

	bool* newCurrentValues = new bool[newOutputCount]{false};
	bool* newCurrentValues2 = new bool[newOutputCount]{false};
	for(int i = 0; i < outputCount; i++){
		if(i >= newOutputCount) break;
		*(newCurrentValues+i) = *(currentValues+i);
		*(newCurrentValues2+i) = *(currentValues2+i);
	}
	delete[] currentValues;
	delete[] currentValues2;
	currentValues = newCurrentValues;
	currentValues2 = newCurrentValues2;
	outputCount = newOutputCount;

	dimensions = {WIDTH, CONNECTOR_OFFSET + CONNECTOR_SIZE + (CONNECTOR_OFFSET+CONNECTOR_SIZE)*std::max(inputCount, outputCount)};
}

bool Node::readSave(const std::string& path){
	std::ifstream reader;
	reader.open(path, std::ios::in|std::ios::binary);
	if(reader.is_open()){
		std::vector<int> nodeIndices;

		int num;
		reader.read((char*) &num, sizeof(int));
		std::cout << "Reading " << num << " nodes." << std::endl;
		for(int i = 0; i < num; i++){
			int type;
			int inputCount;
			int outputCount;
			int condition;
			int texture;
			vec2f pos;
			reader.read((char*) &type, sizeof(int)); // Read padding to random variable and discard it.
			reader.read((char*) &type, sizeof(int));
			reader.read((char*) &inputCount, sizeof(int));
			reader.read((char*) &outputCount, sizeof(int));
			reader.read((char*) &condition, sizeof(int));
			reader.read((char*) &texture, sizeof(int));
			reader.read((char*) &pos, sizeof(vec2f));
			/*std::cout << "type: " << type << std::endl;
			std::cout << "input count: " << inputCount << std::endl;
			std::cout << "output count: " << outputCount << std::endl;
			std::cout << "condition: " << condition << std::endl;
			std::cout << "position: " << pos.x << ", " << pos.y << std::endl;*/
			void (*conditionFunc)(Node* n, bool* values);
			switch(condition){
				case 0:
					conditionFunc = nullptr;
				case 1:
					conditionFunc = nodeAndCondition;
					break;
				case 2:
					conditionFunc = nodeNotCondition;
					break;
				case 3:
					conditionFunc = nodeOrCondition;
					break;
				case 4:
					conditionFunc = nodeXorCondition;
					break;
				default:
					std::cerr << "Error. Invalid function: " << condition << "." << std::endl;
					allNodes.clear();
					reader.close();
					return false;
			}

			Node* n;
			if(type == 0){
				n = new Node(inputCount, outputCount, conditionFunc, texture);
			}
			else if(type == 1){
				n = (Node*) new ButtonNode();
			}
			else{
				std::cerr << "Error. Invalid type: " << type << "." << std::endl;
				allNodes.clear();
				reader.close();
				return false;
			}
			n->pos = pos;

			for(int j = 0; j < inputCount; j++){
				int nodeCount;
				reader.read((char*) &nodeCount, sizeof(int));
				for(int k = 0; k < nodeCount; k++){
					int index;
					int output;
					reader.read((char*) &index, sizeof(int));
					reader.read((char*) &output, sizeof(int));
					/*std::cout << output << std::endl;
					if(n->addInput(j, nullptr, output) == -1){
						std::cerr << "Errrr " << i << ", " << j << ", " << k << ": " << output << std::endl;
					}*/
					(n->inputs+j)->push_back(std::pair<Node*, int>(nullptr, output));
					nodeIndices.push_back(index);
				}
			}

			addToWorld(n);
		}

		int ic = 0;
		for(int i = 0; i < (int)allNodes.size(); i++){
			Node* n = allNodes.at(i);
			for(int j = 0; j < n->inputCount; j++){
				for(int k = 0; k < (int)((((n->inputs)+j))->size()); k++){
					((((n->inputs)+j))->at(k)).first = allNodes.at(nodeIndices.at(ic++));
				}
			}
		}

		reader.close();
		return true;
	}
	else{
		std::cerr << "Could not open file " << path << " for reading." << std::endl;
		reader.close();
		return false;
	}
}
bool Node::writeSave(const std::string& path){
	char padding[5] = "node";
	std::ofstream writer;
	writer.open(path, std::ios::out|std::ios::binary);
	if(writer.is_open()){
		int size = (int)allNodes.size();
		std::cout << "Writing " << size << " nodes." << std::endl;
		writer.write((char*) &size, sizeof(int));
		for(int i = 0; i < (int)allNodes.size(); i++){
			writer.write(padding, 4);
			Node* n = allNodes.at(i);
			writer.write((char*) &n->type, sizeof(int));
			writer.write((char*) &n->inputCount, sizeof(int));
			writer.write((char*) &n->outputCount, sizeof(int));

			int c;
			if(n->condition == nullptr){
				c = 0;
			}
			else if(n->condition == nodeAndCondition){
				c = 1;
			}
			else if(n->condition == nodeNotCondition){
				c = 2;
			}
			else if(n->condition == nodeOrCondition){
				c = 3;
			}
			else if(n->condition == nodeXorCondition){
				c = 4;
			}
			else{
				c = -1;
			}
			writer.write((char*) &c, sizeof(int));

			writer.write((char*) &n->texture, sizeof(int));
			writer.write((char*) &n->pos, sizeof(vec2f));

			for(int j = 0; j < n->inputCount; j++){
				int nodeCount = int((n->inputs+j)->size());
				writer.write((char*) &nodeCount, sizeof(int));
				for(int k = 0; k < nodeCount; k++){
					auto index_i = std::find(allNodes.begin(), allNodes.end(), (n->inputs+j)->at(k).first);
					if(index_i == allNodes.end()){
						std::cerr << "Could not find connector from node " << i << ", input " << j << " node " << k << "." << std::endl;
						index_i = allNodes.begin() + i; // Take input from current node instead of corrupting the save file.
					}
					int index = index_i - allNodes.begin();
					writer.write((char*) &index, sizeof(int));
					writer.write((char*) &((n->inputs+j)->at(k).second), sizeof(int));
				}
			}
		}
		writer.close();
		return true;
	}
	else{
		std::cerr << "Could not open file " << path << " for writing." << std::endl;
		writer.close();
		return false;
	}
}

void Node::onClick(vec2f relMousePos){

}

vec2f& Node::getPos(){
	return pos;
}
vec2f Node::getDimensions(){
	return dimensions;
}
rectangle Node::getRect(){
	return rectangle{pos, dimensions};
}

vec2f Node::getInputConnectorPos(int connector){
	return vec2f{pos.x, pos.y + CONNECTOR_OFFSET + (CONNECTOR_OFFSET+CONNECTOR_SIZE)*connector};
}
vec2f Node::getOutputConnectorPos(int connector){
	return vec2f{pos.x + WIDTH-CONNECTOR_SIZE, pos.y + CONNECTOR_OFFSET + (CONNECTOR_OFFSET+CONNECTOR_SIZE)*connector};
}

int Node::getRectCount(){
	return inputCount + outputCount + 1 + (texture == 0 ? 0 : 1);
}
vertex_rectangle Node::getRect(int i){
	if(i == 0) return vertex_rectangle{pos, dimensions, highlight ? bodyHighlightColor : bodyColor, 0};
	else if(i-1 < inputCount){
		return vertex_rectangle{
			vec2f{pos.x, pos.y + CONNECTOR_OFFSET + (CONNECTOR_OFFSET+CONNECTOR_SIZE)*(i-1)},
			vec2f{CONNECTOR_SIZE, CONNECTOR_SIZE},
			//vec4f{1.0f, 1.0f, 0.0f, 1.0f}
			getInputValue(i-1) ? Node::onColor : Node::offColor,
			0
		};
	}
	else if(i-inputCount-1 < outputCount){
		return vertex_rectangle{
			vec2f{pos.x + WIDTH-CONNECTOR_SIZE, pos.y + CONNECTOR_OFFSET + (CONNECTOR_OFFSET+CONNECTOR_SIZE)*(i-inputCount-1)},
			vec2f{CONNECTOR_SIZE, CONNECTOR_SIZE},
			//vec4f{1.0f, 1.0f, 0.0f, 1.0f}
			getValue(i-inputCount-1) ? Node::onColor : Node::offColor,
			0
		};
	}
	else if(i == inputCount + outputCount + 1 && texture != 0){
		float texSize = Node::WIDTH-2*Node::CONNECTOR_SIZE;
		return vertex_rectangle{
			pos + vec2f{(dimensions.x-texSize)/2, (dimensions.y-texSize)/2},
			vec2f{texSize, texSize},
			vec4f{0.0f},
			texture
		};
	}
	return vertex_rectangle{0.0f};
}

int Node::getLineCount(){
	int lineCount = 0;
	for(int i = 0; i < getInputCount(); i++){
		lineCount += getConnectionCount(i);
	}
	return lineCount;
}
line Node::getLine(int i){
	int c = 0;
	for(int j = 0; j < getInputCount(); j++){
		for(int k = 0; k < getConnectionCount(j); k++){
			if(c == i){
				return line{getInputConnectorPos(j) + vec2f{Node::CONNECTOR_SIZE/2, Node::CONNECTOR_SIZE/2},
							getConnection(j, k).first->getOutputConnectorPos((inputs+j)->at(k).second) + vec2f{Node::CONNECTOR_SIZE/2, Node::CONNECTOR_SIZE/2},
							getConnection(j, k).first->getValue((inputs+j)->at(k).second) ? Node::onColor : Node::offColor};
			}
			c++;
		}
	}
	return line{0.0f};
}

std::vector<Node*>& Node::getAllNodes(){
	return allNodes;
}


ButtonNode::ButtonNode() : Node(0, 1, nullptr) {
	type = 1;
}
bool ButtonNode::getValue(int connector){
	return state;
}
void ButtonNode::onClick(vec2f relMousePos){
	if(relMousePos.x < Node::CONNECTOR_SIZE){
		state = !state;
	}
}
void ButtonNode::resize(int inputCount, int outputCount){

}
int ButtonNode::getRectCount(){
	return 3;
}
vertex_rectangle ButtonNode::getRect(int i){
	if(i == 0) return vertex_rectangle{pos, dimensions, highlight ? bodyHighlightColor : bodyColor, 0};
	else if(i == 1){
		return vertex_rectangle{
			vec2f{pos.x + WIDTH-CONNECTOR_SIZE, pos.y + CONNECTOR_OFFSET},
			vec2f{CONNECTOR_SIZE, CONNECTOR_SIZE},
			state ? Node::onColor : Node::offColor,
			0
		};
	}
	else if(i == 2){
		return vertex_rectangle{pos, vec2f{Node::CONNECTOR_SIZE, dimensions.y}, state ? Node::onColor : Node::offColor, 0};
	}
	return vertex_rectangle{0.0f};
}


// Logic functions

void nodeAndCondition(Node* n, bool* values){
	for(int i = 0; i < n->getInputCount(); i++){
		if(n->getInputValue(i) == false){
			for(int j = 0; j < n->getOutputCount(); j++){
				*(values+j) = false;
			}
			return;
		}
	}
	for(int j = 0; j < n->getOutputCount(); j++){
		*(values+j) = true;
	}
}
void nodeNotCondition(Node* n, bool* values){
	*(values+0) = !n->getInputValue(0);

}
void nodeXorCondition(Node* n, bool* values){
	bool val = false;
	for(int i = 0; i < n->getInputCount(); i++){
		if(val != n->getInputValue(i)) val = true;
		else val = false;
	}
	for(int j = 0; j < n->getOutputCount(); j++){
		*(values+j) = val;
	}
}
void nodeOrCondition(Node* n, bool* values){
	for(int i = 0; i < n->getInputCount(); i++){
		if(n->getInputValue(i) == true){
			for(int j = 0; j < n->getOutputCount(); j++){
				*(values+j) = true;
			}
			return;
		}
	}
	for(int j = 0; j < n->getOutputCount(); j++){
		*(values+j) = false;
	}
}
void nodeAdder(Node* n, bool* values){
	int n1 = 0;
	int n2 = 0;
	for(int i = 0; i < 8; i++){
		n1 *= 2;
		n1 += n->getInputValue(7-i) ? 1 : 0;
	}
	for(int i = 0; i < 8; i++){
		n2 *= 2;
		n2 += n->getInputValue(7-i+8) ? 1 : 0;
	}
	int s = n1 + n2;
	for(int i = 0; i < 10; i++){
		*(values+i) = (s & 1) == 1 ? true : false;
		s /= 2;
	}
}
