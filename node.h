#ifndef NODE_H
#define NODE_H

#include "helper.h"

#include <vector>
#include <utility>

class Node{
	public:
		Node(int inputCount, int outputCount, void (*condition)(Node* n, bool* values), int texture = 0);
		virtual ~Node();

		Node* createNew();

		static void addToWorld(Node* node);
		static void removeFromWorld(Node* node);
		static std::vector<Node*>& getAllNodes();

		static const constexpr float WIDTH = 1.0f;
		static const constexpr float CONNECTOR_SIZE = 1.0f/4.0f;
		static const constexpr float CONNECTOR_OFFSET = 0.5f-CONNECTOR_SIZE;

		static const constexpr vec4f onColor = vec4f{0.0f, 1.0f, 0.0f, 1.0f};
		static const constexpr vec4f offColor = vec4f{1.0f, 0.0f, 0.0f, 1.0f};
		static const constexpr vec4f bodyColor = vec4f{0.5f, 0.5f, 0.5f, 1.0f};
		static const constexpr vec4f bodyHighlightColor = vec4f{0.6f, 0.6f, 0.5f, 1.0f};

		void update(); // Calculate out value(s)
		void setOut(); // Set calculated out value(s)

		int addInput(int inConnector, Node* node, int outConnector); // Connects "node"'s output "outConnector" to input "inConnector".
		int addInput(int inConnector, std::pair<Node*, int> connection);
		bool hasInput(int inConnector, Node* node, int outConnector); // Returns if "node"'s output "outConnector" is connected to input "inConnector".
		bool hasInput(Node* node); // Returns if this node takes input from "n".
		int removeInput(int inConnector, Node* node, int outConnector); // Remove "node"'s output "outConnector" from input "inConnector".
		int removeInput(Node* node);
		virtual bool getInputValue(int connector);
		virtual bool getValue(int connector); // Get current value.

		int getInputCount();
		int getOutputCount();

		int getType();

		int getConnectionCount(int connector);
		//Node* getNode(int connector, int count);
		std::pair<Node*, int> getConnection(int connection, int count);

		virtual void resize(int inputCount, int outputCount);

		static bool readSave(const std::string& path);
		static bool writeSave(const std::string& path);

		virtual void onClick(vec2f relMousePos);

		vec2f& getPos();
		vec2f getDimensions();
		rectangle getRect();

		virtual vec2f getInputConnectorPos(int connector);
		virtual vec2f getOutputConnectorPos(int connector);

		/*rectangle getRect();
		rectangle getInputRect(int connector);
		rectangle getOutputRect(int connector);
		line getLine(int connector, int count);*/

		virtual int getRectCount();
		virtual vertex_rectangle getRect(int i);
		virtual int getLineCount();
		virtual line getLine(int i);

		vec2f pos;
		bool highlight = false;

	protected:
		int inputCount;
		std::vector<std::pair<Node*, int>>* inputs;

		int outputCount;
		bool* currentValues;
		bool* currentValues2;

		int type;

		void (*condition)(Node* n, bool* values);

		vec2f dimensions;

		int texture;

	private:
		static std::vector<Node*> allNodes;
};

class ButtonNode : Node{
	public:
		ButtonNode();
		bool getValue(int connector);
		void onClick(vec2f relMousePos);
		void resize(int inputCount, int outputCount);
		int getRectCount();
		vertex_rectangle getRect(int i);
	private:
		bool state = false;
};

// Logic functions

void nodeAndCondition(Node* n, bool* values);
void nodeNotCondition(Node* n, bool* values);
void nodeXorCondition(Node* n, bool* values);
void nodeOrCondition(Node* n, bool* values);
void nodeAdder(Node* n, bool* values);

#endif // NODE_H
