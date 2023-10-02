#include "json_builder.h"

namespace json {
    
    
    KeyItemContext Builder::Key(std::string key) {
        if (nodes_stack_.back()->IsDict() && !key_dict) {
            key_ = std::move(key);
            key_dict = true;
        }
        else throw std::logic_error("Wrong map key: " + key);
        return KeyItemContext(*this);
    }
    
    Builder& Builder::Value(Node::Value&& value) {
        if (root_.IsNull()) {
            root_ = GetNode(value);
            return *this;
        }
        auto* back_node = nodes_stack_.back();
        
        if (back_node->IsDict()) {
            
            if (!key_dict) {
                throw std::logic_error("Could not call for dict without key");
            }
            std::get<Dict>(nodes_stack_.back()->GetValue()).emplace(key_, GetNode(value));
            
            //const_cast<Dict&>(back_node->AsDict()).emplace(key_, GetNode(value));
            key_dict = false;
        }
        else if (back_node->IsArray()) {
            
            std::get<Array>(back_node->GetValue()).emplace_back(GetNode(value));
            //const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(GetNode(value));
        }
        else throw std::logic_error("Value() called in unknow container");
        return *this;
    }
    
    DictItemContext Builder::StartDict() {
        Value(Dict{});
        AddStack();
        return DictItemContext(*this);
    }
    
   ArrayItemContext Builder::StartArray() {
        Value(Array{});
        AddStack();
        return ArrayItemContext(*this);
    }
    
    Builder& Builder::EndDict() {
        auto* back_node = nodes_stack_.back();
        
        if (!back_node->IsDict()) {
            throw std::logic_error("Prev node is not a Dict");
        }
        nodes_stack_.pop_back();
        return *this;
    }
    
    Builder& Builder::EndArray() {
        auto* back_node = nodes_stack_.back();
        
        if (!back_node->IsArray()) {
            throw std::logic_error("Prev node is not an Array");
        }
        nodes_stack_.pop_back();
        return *this;
    }
    
    Node Builder::Build() {
         if (root_.IsNull() || nodes_stack_.size() > 1) {
             throw std::logic_error("Wrong Build()");
         }
        return root_;
    }
    
    Node Builder::GetNode(Node::Value value) {
        Node node = std::visit([](auto val){
            return Node(val);
        }, value);
        return node;
    }
    
    void Builder::AddStack() {
        if (nodes_stack_.empty()) {
            nodes_stack_.emplace_back(&root_);
            return;
        }
        auto* back_node = nodes_stack_.back();

        if (back_node->IsDict()) {
            auto& node = std::get<Dict>(back_node->GetValue()).at(key_);
            nodes_stack_.emplace_back(&node);
            return;
        }
        if (back_node->IsArray()) {
            auto& node = std::get<Array>(back_node->GetValue()).back();
            nodes_stack_.push_back(&node);
            return;
        }
        throw std::logic_error("invalid stack");
    }
        
    
    KeyItemContext BaseContext::Key(std::string key)
    {
        return KeyItemContext(builder_.Key(std::move(key)));
    }

    Builder& BaseContext::Value(Node::Value&& value)
    {
        return builder_.Value(std::move(value));
    }

    DictItemContext BaseContext::StartDict()
    {
        return DictItemContext(builder_.StartDict());
    }

    Builder& BaseContext::EndDict()
    {
        return builder_.EndDict();
    }

    ArrayItemContext BaseContext::StartArray()
    {
        return ArrayItemContext(builder_.StartArray());
    }

    Builder& BaseContext::EndArray()
    {
        return builder_.EndArray();
    }


    ArrayItemContext ArrayItemContext::Value(Node::Value value)
    {
        return ArrayItemContext(BaseContext::Value(std::move(value)));
    }

    DictItemContext KeyItemContext::Value(Node::Value value)
    {
        return DictItemContext(BaseContext::Value(std::move(value)));
    }

}