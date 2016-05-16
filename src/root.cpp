#include "root.h"
#include "hookmanager.h"
#include "clientmanager.h"
#include "attribute.h"
#include "ipc-protocol.h"

#include <memory>
#include <stdexcept>

namespace herbstluft {

std::shared_ptr<Root> Root::root_;

std::shared_ptr<Root> Root::create() {
    if (root_)
        throw std::logic_error("Redundant root node creation!");
    root_ = std::make_shared<Root>();
    return root_;
}

void Root::destroy() {
    root_->children_.clear(); // avoid possible circular shared_ptr dependency
    root_.reset();
}

Root::Root() : Object("root")
{
    addChild(std::make_shared<HookManager>());
    addChild(std::make_shared<ClientManager>());
}

std::shared_ptr<HookManager> Root::hooks()
{
    return root_->child<HookManager>("hooks");
}

std::shared_ptr<ClientManager> Root::clients() {
    return root_->child<ClientManager>("clients");
}

int Root::cmd_ls(Input in, Output out)
{
    in.shift();
    if (in.empty()) {
        root_->ls(out);
    } else {
        Path p(in.front());
        root_->ls(p, out);
    }

    return 0;
}

Attribute* Root::getAttribute(std::string path, Output output) {
    auto attr_path = Object::splitPath(path);
    auto child = Root::get()->child<Object>(attr_path.first);
    if (!child) {
        output << "No such object " << attr_path.first.join('.') << std::endl;
        return NULL;
    }
    Attribute* a = child->attribute(attr_path.second);
    if (!a) {
        output << "Object " << attr_path.first.join('.')
               << " has no attribute \"" << attr_path.second << "\""
               << std::endl;
        return NULL;
    }
    return a;
}

int print_object_tree_command(int argc, char* argv[], Output output) {
    Root::get()->printTree(output);
    return 0;
}

int attribute_get_command(ArgList args, Output output) {
    if (args.size() >= 2) {
        Attribute* a = Root::get()->getAttribute(args[1], output);
        if (!a) return HERBST_INVALID_ARGUMENT;
        // TODO: Why is this always the empty string?
        output << a->str() << std::endl;
        return 0;
    }
    return HERBST_NEED_MORE_ARGS;
}

}
