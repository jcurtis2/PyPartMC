/*##################################################################################################
# This file is a part of PyPartMC licensed under the GNU General Public License v3 (LICENSE file)  #
# Copyright (C) 2022 University of Illinois Urbana-Champaign                                       #
# Authors: https://github.com/open-atmos/PyPartMC/graphs/contributors                              #
##################################################################################################*/

#pragma once

#include <iostream>
#include <sstream>
#include <set>
#include <stack>
#include <map>
#include "nlohmann/json.hpp"
#include <tcb/span.hpp>
#include <bpstd/string_view.hpp>
#include "input_guard.hpp"

struct JSONResource {
  private:
    std::set<std::string> vars;
    const nlohmann::json *json;
    std::stack<const nlohmann::json*> json_parent;

    std::unique_ptr<InputGuard> input_guard_ptr;

    void warn(const std::exception &exception) {
        std::cerr << "WARN: " << exception.what() << std::endl;
//        assert(false);
    }

  protected:
    size_t index = 0, named_array_read_count = 0;

    JSONResource() {}

    JSONResource(const nlohmann::json &json) {
        this->set_current_json_ptr(&json);
        for (auto &entry : this->json->items()) {
            this->vars.insert(entry.key());
        }

        input_guard_ptr = std::make_unique<InputGuard>(json);
    };

    void set_current_json_ptr(const nlohmann::json *ptr) {
        this->json = ptr;
    }

    auto dump() const {
        return this->json->dump();
    }

    template <class T>
    std::string next_dict_key(T last_key, const T key_cond) const noexcept {
        std::string key = "", prev_key = "";
        if (last_key == key_cond)
            last_key = "";

        for (const auto& item : this->json->items()) {
            if (this->json->is_array()) {
                for (auto &entry : item.value().items()) {
                    if (prev_key == last_key) {
                        key = entry.key();
                        break;
                    }
                    else
                        prev_key = entry.key();
                }
            } else {
                key = item.key();
            }
        }

        input_guard_ptr->update_dict_key(key);

        return key;
    }

  public:
    virtual ~JSONResource() {}

    auto n_named_array_read_count() noexcept {
        return this->named_array_read_count;
    }

    void zoom_in(const bpstd::string_view &sub) noexcept {
        auto it = this->json->is_array() 
            ? this->json->at(this->json->size()-1).find(sub)
            : this->json->find(sub);

        assert(*it != NULL);
        this->json_parent.push(this->json);

        if (sub == "dist")  // TODO #112: do better than hardcoding "dist"...
            this->set_current_json_ptr(&(it->at(this->n_elements(sub) - this->index--)));
        else
            this->set_current_json_ptr(&(*it));

        this->named_array_read_count = 0;
    }

    void zoom_out() noexcept {
        assert(this->json_parent.size() != 0);
        this->set_current_json_ptr(this->json_parent.top());
        this->json_parent.pop();
    }

    auto zoom_level() const noexcept {
        auto level = this->json_parent.size();
        return level;
    }

    auto begin() noexcept {
        return this->json->begin();
    }

    auto first_field_name() noexcept {
        // TODO #112: handle errors
        std::string name = "";
        assert(this->json->size() > 0);
        assert(this->json->begin()->size() > 0);
        for (auto &entry : this->json->at(this->named_array_read_count++).items())
        {
            name = entry.key();
        }
        if (name == "")
            assert(false);
        return name;
    }

    std::size_t n_elements(const bpstd::string_view &name) const noexcept {
        std::size_t n_elem = 0;
        for (auto i=0u; i<this->json->size(); ++i) {
            for (auto &entry : this->json->at(i).items()) {
                if (entry.key() == name)
                    n_elem = entry.value().size();
            }
        }
        return n_elem;
    }

    auto n_numeric_array_entries() noexcept {
        auto count = 0u;
        for (auto i=0u; i<this->json->size(); ++i) {
            assert(this->json->at(i).is_object());
            assert(this->json->at(i).size() == 1);
            for (auto &entry : this->json->at(i).items()) 
                if (entry.value().is_array() && (entry.value().size() == 0 || entry.value().at(0).is_number()))
                    ++count;
        }
        return count;
    }

    template <typename T>
    void read_value(
        const bpstd::string_view name,
        T *var
    ) {
        *var = this->find(name)->get<T>();
    }

    void read_str(
        const bpstd::string_view &name,
        char* var_data,
        int* var_size
    ) noexcept {
        auto it = this->find(name);
        if (it == this->json->end())
        {
            assert(false);
            // TODO #112
            return;
        }
        auto value = it->is_array()
            ? name
            : it->begin()->get<bpstd::string_view>();  // TODO #112: is this path used anywhere?
        if ((int)value.size() > *var_size) {
            std::ostringstream oss;
            oss << "provided entry \"" << name << "\" has too many characters";
            this->warn(std::invalid_argument(oss.str()));
        }
        for (auto i = 0u; i < value.size(); ++i)
            var_data[i] = value[i];
        var_size[0] = value.size();
    }

    template <typename T1, typename T2>
    void read_arr(
        const T1 &name,
        const tcb::span<T2> &values
    ) noexcept {
        for (auto i=0u; i<this->json->size(); ++i) {
            for (auto &entry : this->json->at(i).items()) {
                if (entry.key() == name) {
                    for (auto j = 0u; j < values.size(); ++j)
                        values[j] = entry.value().at(j);
                    return;
                }
            }
        }
        // TODO #112: check size
    }

    auto varid(const std::string& name) noexcept {
        auto it = this->vars.find(name);
        if (it == this->vars.end()) {
            this->warn(std::logic_error("var not found!"));
        }
        return std::distance(this->vars.begin(), it);
    }

    template <typename T>
    auto find(const T& entry) noexcept -> decltype(this->json->end()) {
        auto it = this->json->find(entry);
        if (it == this->json->end()) {
            std::ostringstream oss;
            oss << "provided data is missing the \"" << entry << "\" entry";
            this->warn(std::invalid_argument(oss.str()));
        }
        return it;
    }

    InputGuard *get_input_guard_ptr() {
        return input_guard_ptr.get();
    }

    virtual std::string str() const = 0;

    virtual bool read_line(std::string &name, std::string &data) = 0;

    virtual int read_line_data_size_and_start_enumerating() = 0;
};

struct InputJSONResource: JSONResource {
  private:
    std::string key_cond, key_name;
    std::string last_read_line_key = "";
    std::size_t max_zoom_level;

  public:
    InputJSONResource(
        const nlohmann::json &json,
        const std::string key_cond = "",
        const std::string key_name = "",
        const std::size_t max_zoom_level = 3
    ) : JSONResource(json), key_cond(key_cond), key_name(key_name), max_zoom_level(max_zoom_level)
    {
    }

    std::string str() const {
        throw std::logic_error("str() called on InputJSONResource!");
    }

    bool read_line(std::string &name, std::string &data) {
        bool subsequent_record = false;
        if (this->zoom_level() == this->max_zoom_level) {
            this->zoom_out();

            auto key = this->next_dict_key(this->last_read_line_key, this->key_cond);
            if (key == "") {
                if (this->index == 0)
                    this->last_read_line_key = "";
                else
                    this->last_read_line_key = this->key_cond;
                return true;
            }
            else
                subsequent_record = true;
        }

        auto key = this->next_dict_key(this->last_read_line_key, this->key_cond);
        if (subsequent_record || (this->key_name != "" && (this->key_cond == this->last_read_line_key))) {
            name = this->key_name;
            this->zoom_in(key);
        } else {
            name = key;
        }
        data = key;
        this->last_read_line_key = key;

        return false;
    }

    int read_line_data_size_and_start_enumerating() {
        if (this->zoom_level() == this->max_zoom_level - 2) {
            this->index = this->n_elements(this->key_cond);
            return this->index;
        }
        return 1;
    }

    static bool unique_keys(const nlohmann::json &json) {
        std::set<std::string> keys;
        for (auto i=0u; i<json.size(); ++i) {
            for (auto &entry : json.at(i).items()) {
                if (keys.find(entry.key()) != keys.end())
                    return false;
                keys.insert(entry.key());
            }
        }
        return true;
    }
};

struct OutputJSONResource: JSONResource {
    std::unique_ptr<nlohmann::json> guard;

    OutputJSONResource() : guard(std::make_unique<nlohmann::json>()) {
        this->set_current_json_ptr(this->guard.get());
    }

    std::string str() const {
        return this->dump();
    }

    bool read_line(std::string &, std::string &) {
        throw std::logic_error("read_line() called on OutputJSONResource!");
    }

    int read_line_data_size_and_start_enumerating() {
        throw std::logic_error("read_line_data_size_and_start_enumerating() called on OutputJSONResource!");
    }
};

std::unique_ptr<JSONResource> &json_resource_ptr();

template <typename T>
struct JSONResourceGuard {
    JSONResourceGuard() {
        json_resource_ptr() = std::make_unique<T>();
    }

    JSONResourceGuard(const nlohmann::json & json) {
        json_resource_ptr() = std::make_unique<T>(json);
    }

    JSONResourceGuard(
        const nlohmann::json & json,
        const std::string key_cond,
        const std::string key_name
    ) {
        json_resource_ptr() = std::make_unique<T>(json, key_cond, key_name);
    }

    JSONResourceGuard(
        const nlohmann::json & json,
        const std::string key_cond,
        const std::string key_name,
        const int max_zoom_level
    ) {
        json_resource_ptr() = std::make_unique<T>(json, key_cond, key_name, max_zoom_level);
    }

    ~JSONResourceGuard() {
        json_resource_ptr().reset();
    }

    void check_parameters() {
        json_resource_ptr()->get_input_guard_ptr()->check_used_inputs();
    }
};

