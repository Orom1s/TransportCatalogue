#pragma once

#include "json.h"


namespace json {

class BaseContext;
class KeyItemContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    Builder() = default;
    KeyItemContext Key(std::string key);
    Builder& Value(Node::Value&& value);
    DictItemContext StartDict();
    Builder& EndDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
    Node Build();
    
private:
    Node GetNode(Node::Value value);
    void AddStack();

    Node root_{ nullptr };
    std::vector<Node*> nodes_stack_;
    std::string key_{};
    bool key_dict = false;
};

class BaseContext {
public:
    BaseContext(Builder& builder) : builder_(builder) {}
    KeyItemContext Key(std::string key);
    Builder& Value(Node::Value&& value);
    DictItemContext StartDict();
    Builder& EndDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

private:
    Builder& builder_;
};


class KeyItemContext : public BaseContext {
public:
    KeyItemContext(BaseContext base) : BaseContext(base) {}

    DictItemContext Value(Node::Value value);
    Builder& EndArray() = delete;
    Builder& EndDict() = delete;
    KeyItemContext Key(std::string key) = delete;
};

class DictItemContext : public BaseContext {
public:
    DictItemContext(BaseContext base) : BaseContext(base) {}
    DictItemContext StartDict() = delete;
    Builder& EndArray() = delete;
    ArrayItemContext StartArray() = delete;
    Builder& Value(Node::Value&& value) = delete;
};

class ArrayItemContext : public BaseContext {
public:
    ArrayItemContext(BaseContext base) : BaseContext(base) {}
    KeyItemContext Key(std::string key) = delete;
    Builder& EndDict() = delete;
    ArrayItemContext Value(Node::Value value);

};
  
}