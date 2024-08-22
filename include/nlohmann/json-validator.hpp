#ifndef INCLUDE_NLOHMANN_JSON_VALIDATOR_HPP_
#define INCLUDE_NLOHMANN_JSON_VALIDATOR_HPP_

#include <nlohmann/json.hpp>
#include <unordered_set>

namespace nlohmann::detail {
  void concat_stream_impl(std::stringstream &) {
    /* recursion breaker */
  }

  template<typename TArg, typename ...TArgs>
  void concat_stream_impl(std::stringstream &ss, TArg &&arg, TArgs &&...args) {
    ss << std::forward<TArg>(arg);
    concat_stream_impl(ss, std::forward<TArgs>(args)...);
  }

  template<typename TArg, typename ...TArgs>
  void concat_stream_impl(std::stringstream &ss, const TArg& arg, TArgs &&...args) {
    ss << arg;
    concat_stream_impl(ss, std::forward<TArgs>(args)...);
  }

  template<typename ...TArgs>
  std::string concat_stream(TArgs &&...args) {
    std::stringstream ss;
    concat_stream_impl(ss, std::forward<TArgs>(args)...);
    return ss.str();
  }

  template<typename _Container>
  std::string to_string_streamed(const _Container &__container, const std::string &__delimiter = ", ") {
    std::stringstream ss;
    bool first = true;
    for (auto &&element : __container) {
      if (first) {
        first = false;
        ss << element;
      } else {
        ss << __delimiter << element;
      }
    }
    return ss.str();
  }
}

namespace nlohmann::validation {

enum struct value_t : std::uint8_t {
  undefined       = (0 << 0),
  null            = (1 << 0),
  object          = (1 << 1),
  array           = (1 << 2),
  string          = (1 << 3),
  boolean         = (1 << 4),
  number_integer  = (1 << 5),
  number_unsigned = (1 << 6),
  number_float    = (1 << 7),
  number          = number_integer | number_unsigned | number_float,
};

inline const char * value_type_name(value_t vt) noexcept {
  switch (vt) {
    case value_t::null:            return "null";
    case value_t::object:          return "object";
    case value_t::array:           return "array";
    case value_t::string:          return "string";
    case value_t::boolean:         return "boolean";
    case value_t::number_integer:  return "number_integer";
    case value_t::number_unsigned: return "number_unsigned";
    case value_t::number_float:    return "number_float";
    default: {
      if (static_cast<std::uint8_t>(vt) & static_cast<std::uint8_t>(value_t::number))
        return "number";
      return "undefined";
    }
  }
}

constexpr inline detail::value_t to_detail_value_type(value_t vt) noexcept {
  switch (vt) {
    case value_t::null:            return detail::value_t::null;
    case value_t::object:          return detail::value_t::object;
    case value_t::array:           return detail::value_t::array;
    case value_t::string:          return detail::value_t::string;
    case value_t::boolean:         return detail::value_t::boolean;
    case value_t::number_integer:  return detail::value_t::number_integer;
    case value_t::number_unsigned: return detail::value_t::number_unsigned;
    case value_t::number_float:    return detail::value_t::number_float;
    default:                       return detail::value_t::discarded;
  }
}

constexpr inline value_t to_value_type(detail::value_t dvt) noexcept {
  switch (dvt) {
    case detail::value_t::null:            return value_t::null;
    case detail::value_t::object:          return value_t::object;
    case detail::value_t::array:           return value_t::array;
    case detail::value_t::string:          return value_t::string;
    case detail::value_t::boolean:         return value_t::boolean;
    case detail::value_t::number_integer:  return value_t::number_integer;
    case detail::value_t::number_unsigned: return value_t::number_unsigned;
    case detail::value_t::number_float:    return value_t::number_float;
    default:                               return value_t::undefined;
  }
}

constexpr inline bool operator==(value_t vt, detail::value_t dvt) {
  return static_cast<std::uint8_t>(vt) & static_cast<std::uint8_t>(to_value_type(dvt));
}

constexpr inline bool operator!=(value_t vt, detail::value_t dvt) {
  return !(vt == dvt);
}

struct error_t {
  error_t(std::string message)
    : _message{std::move(message)}
  {}

  const auto & message() const { return _message; }

  friend std::ostream & operator<<(std::ostream &stream, const error_t &error) {
    return stream << error.message();
  }

private:
  std::string _message;
};

using errors_t = std::vector<error_t>;
using errors_map_t = std::map<json::json_pointer, errors_t>;
using ignored_t = std::vector<json::json_pointer>;

struct processor;
struct errors_collector_t {
  void emplace(std::string message) {
    auto &&[it, _0] = _errors.try_emplace(_pointer);
    auto &&[_1, errors] = *it;
    errors.emplace_back(std::move(message));
  }

  template<typename ...TArgs>
  void emplace_streamed(TArgs &&...args) {
    emplace(detail::concat_stream(std::forward<TArgs>(args)...));
  }

private:
  friend processor;
  errors_collector_t(const json::json_pointer &pointer, errors_map_t &errors)
    : _pointer{pointer}
    , _errors{errors}
  {}

private:
  const json::json_pointer &_pointer;
  errors_map_t             &_errors;
};

enum struct rule_t {
  size,
  type,
  custom
};

struct rule_base {
  constexpr virtual rule_t type() const { return rule_t::custom; }
  virtual bool operator() (const nlohmann::json &json, errors_collector_t &errors) const = 0;
};

struct of_size_rule : public rule_base {
  of_size_rule(size_t size) : _size{size} {}

  bool operator() (const nlohmann::json &json, errors_collector_t &errors) const override {
    if (json.size() == _size)
      return true;

    errors.emplace_streamed("not of size: ", _size);
    return false;
  }

  constexpr rule_t type() const override {
    return rule_t::size;
  }

private:
  size_t _size;
};

struct of_type_rule : public rule_base {
  of_type_rule(value_t type)
    : _type{std::move(type)}
  {}

  bool operator() (const nlohmann::json &json, errors_collector_t &errors) const override {
    if (json.type() == _type)
      return true;

    errors.emplace_streamed("not of type: ", value_type_name(_type));
    return false;
  }

  constexpr rule_t type() const override {
    return rule_t::type;
  }

private:
  value_t _type;
};

template<typename T>
struct in_range_rule : public of_type_rule {
  in_range_rule(std::optional<T> min, std::optional<T> max, value_t type)
    : of_type_rule{std::move(type)}
    , _min{std::move(min)}
    , _max{std::move(max)}
  {}

  bool operator() (const nlohmann::json & json, errors_collector_t &errors) const override {
    if (!of_type_rule::operator()(json, errors))
      return false;

    auto value = json.get<T>();
    if ((_min && value < _min) || (_max && value > _max)) {
      errors.emplace_streamed(
        "not in range: [",
          (_min ? std::to_string(*_min) : "inf"), ", ",
          (_max ? std::to_string(*_max) : "inf"),
        "]"
      );
      return false;
    }
    return true;
  }

protected:
  std::optional<T> _min;
  std::optional<T> _max;
};

template<typename T>
struct in_set_rule : public rule_base {
  in_set_rule(std::initializer_list<T> values)
    : _values{std::move(values)}
  {}

  bool operator() (const nlohmann::json & json, errors_collector_t &errors) const override {
    if (!_values.count(json.get<T>())) {
      errors.emplace_streamed("not in set: [", detail::to_string_streamed(_values), "]");
      return false;
    }
    return true;
  }

protected:
  std::unordered_set<T> _values;
};

struct processor {
private:
  processor(processor *owner, const std::string &name)
    : _owner{owner}
    , _pointer{owner->_pointer / name}
    , _optional{false}
  {}

public:
  processor()
    : _owner{nullptr}
    , _optional{false}
  {}

  processor & with_value(const std::string &name) {
    auto &&[it, _0] = _sub_processors.try_emplace(name, new processor(this, name));
    return *it->second;
  }

  processor & with_object(const std::string &name) {
    return with_value(name).of_type(value_t::object);
  }

  processor & with_array(const std::string &name) {
    return with_value(name).of_type(value_t::array);
  }

  processor & with_string(const std::string &name) {
    return with_value(name).of_type(value_t::string);
  }

  processor & with_boolean(const std::string &name) {
    return with_value(name).of_type(value_t::boolean);
  }

  processor & with_number(const std::string &name) {
    return with_value(name).of_type(value_t::number);
  }


  template<typename TRule, typename ...TArgs>
  processor & with_rule(TArgs &&...args) {
    _rules.emplace_back(new TRule{std::forward<TArgs>(args)...});
    return *this;
  }

  processor & of_type(value_t type) {
    return with_rule<of_type_rule>(type);
  }

  processor & of_size(size_t size) {
    return with_rule<of_size_rule>(size);
  }

  template<typename T>
  processor & in_range(std::optional<T> min, std::optional<T> max, value_t type = value_t::number) {
    return with_rule<in_range_rule<T>>(std::move(min), std::move(max), std::move(type));
  }

  template<typename T>
  processor & in_set(std::initializer_list<T> init) {
    return with_rule<in_set_rule<T>>(std::move(init));
  }

  processor & back() {
    return _owner ? *_owner : *this;
  }

  processor & optional() {
    _optional = true;
    return *this;
  }

  bool exec(const nlohmann::json &json, errors_map_t &errors) {
    errors_collector_t collector{_pointer, errors};

    for (auto &&rule : _rules)
      (*rule)(json, collector);

    for (auto &&[sub_key, sub_processor] : _sub_processors) {
      if (!json.contains(sub_key)) {
        if (!sub_processor->_optional)
          collector.emplace_streamed("missing required value: ", std::quoted(sub_key));

        continue;
      }
      auto &&sub_json = json.at(sub_key);
      sub_processor->exec(sub_json, errors);
    }
    return errors.empty();
  }

  bool exec(const nlohmann::json &json) {
    errors_map_t errors;
    return exec(json, errors);
  }

  bool exec(const nlohmann::json &json, errors_map_t &errors, ignored_t &ignored) {
    errors_collector_t collector{_pointer, errors};
    for (auto &&rule : _rules)
      (*rule)(json, collector);

    for (auto &&[sub_key, sub_json] : json.items()) {
      if (sub_key.empty())
        continue;

      auto it = _sub_processors.find(sub_key);
      if (it == _sub_processors.end()) {
        ignored.push_back(_pointer / sub_key);
        continue;
      }
      it->second->exec(sub_json, errors, ignored);
    }
    for (auto &&[sub_key, sub_processor] : _sub_processors) {
      if (sub_processor->_executed)
        continue;
      if (sub_processor->_optional)
        continue;
      collector.emplace_streamed("missing required value: ", std::quoted(sub_key));
    }
    _executed = true;
    return errors.empty();
  }

private:
  using rules = std::vector<std::unique_ptr<rule_base>>;
  using sub_processors = std::unordered_map<std::string, std::unique_ptr<processor>>;

private:
  processor          *_owner;
  json::json_pointer  _pointer;
  bool                _optional;
  bool                _executed;
  rules               _rules;
  sub_processors      _sub_processors;
};

inline processor value() {
  return processor{};
}

inline processor object() {
  processor p;
  p.of_type(value_t::object);
  return p;
}

inline processor array() {
  processor p;
  p.of_type(value_t::array);
  return p;
}

inline processor string() {
  processor p;
  p.of_type(value_t::string);
  return p;
}

inline processor boolean() {
  processor p;
  p.of_type(value_t::boolean);
  return p;
}

inline processor number() {
  processor p;
  p.of_type(value_t::number);
  return p;
}

} // namespace nlohmann::validation

#endif//INCLUDE_NLOHMANN_JSON_VALIDATOR_HPP_
