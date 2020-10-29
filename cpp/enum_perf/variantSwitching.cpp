#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <variant>
#include <vector>

struct Point_2D
{
  double x;
  double y;
};

struct Transform_no_change
{
};

struct Transform_rigid_change
{
  double cosa;
  double sina;
  Point_2D offset;
};

struct Transform_scale
{
  double multiplier;
};

struct Transform_extra_vertex
{
  uint64_t insertion_idx;
  Point_2D vertex;
};


using Transform = std::variant<Transform_no_change,
                               Transform_rigid_change,
                               Transform_scale,
                               Transform_extra_vertex>;

struct Transform_union
{
  union
  {
    Transform_no_change    no_change;
    Transform_rigid_change rigid_change;
    Transform_scale        scale;
    Transform_extra_vertex extra_vertex;
  };

  enum class Type : unsigned char
  {
    no_change,
    rigid_change,
    scale,
    extra_vertex
  } type;
};

auto transform_no_change() {
    return [] {
        return 1.0;
    };
}

auto transform_rigid_change(double cosa, double sina, const Point_2D& offset) {
    return [=] {
        return cosa;
    };
}

auto transform_scale(double multiplier) {
    return [=] {
        return multiplier;
    };
}

auto transform_extra_vertex(uint64_t insertion_idx, const Point_2D& vertex) {
    return [=] {
        return vertex.x;
    };
}

using DoTransform_no_change = decltype(transform_no_change());
using DoTransform_rigid_change = decltype(transform_rigid_change({},{},{}));
using DoTransform_scale = decltype(transform_scale({}));
using DoTransform_extra_vertex = decltype(transform_extra_vertex({},{}));

using DoTransform = std::variant<DoTransform_no_change,
                               DoTransform_rigid_change,
                               DoTransform_scale,
                               DoTransform_extra_vertex>;

double calc_with_union(const Transform_union& transform)
{
  switch (transform.type)
  {
    case Transform_union::Type::no_change:
      return 1;
    case Transform_union::Type::rigid_change:
      return transform.rigid_change.cosa;
    case Transform_union::Type::scale:
      return transform.scale.multiplier;
    case Transform_union::Type::extra_vertex:
      return transform.extra_vertex.vertex.x;
  }
  return 0;
}

double calc_with_if(const Transform& transform)
{
  if (const auto& change = std::get_if<Transform_no_change>(&transform))
    return 1;

  if (const auto& change = std::get_if<Transform_rigid_change>(&transform))
    return change->cosa;

  if (const auto& change = std::get_if<Transform_scale>(&transform))
    return change->multiplier;

  if (const auto& change = std::get_if<Transform_extra_vertex>(&transform))
    return change->vertex.x;

  return 0;
}

template <typename TVariant, typename TType, size_t idx = 0>
constexpr size_t variant_index()
{
  if constexpr (idx == std::variant_size_v<TVariant>) //NOLINT(readability-braces-around-statement)
  {
    return idx;
  }
  else if constexpr (std::is_same_v<std::variant_alternative_t<idx, TVariant>, TType>) //NOLINT(readability-braces-around-statement)
  {
    return idx;
  }
  else //NOTE: `if constexpr` seems to need the `else`s
  {
    return variant_index<TVariant, TType, idx+1>();
  }
}

double calc_with_switch(const Transform& transform)
{
  switch (transform.index())
  {
    case variant_index<Transform, Transform_no_change>():
      return 1;

    case variant_index<Transform, Transform_rigid_change>():
      return std::get<Transform_rigid_change>(transform).cosa;

    case variant_index<Transform, Transform_scale>():
      return std::get<Transform_scale>(transform).multiplier;

    case variant_index<Transform, Transform_extra_vertex>():
      return std::get<Transform_extra_vertex>(transform).vertex.x;
  }
  return 0;
}

double calc_with_switch_if(const Transform& transform)
{
  switch (transform.index())
  {
    case variant_index<Transform, Transform_no_change>():
      return 1;

    case variant_index<Transform, Transform_rigid_change>():
      return std::get_if<Transform_rigid_change>(&transform)->cosa;

    case variant_index<Transform, Transform_scale>():
      return std::get_if<Transform_scale>(&transform)->multiplier;

    case variant_index<Transform, Transform_extra_vertex>():
      return std::get_if<Transform_extra_vertex>(&transform)->vertex.x;
  }
  return 0;
}

double calc_with_visit(const Transform& transform)
{
  return std::visit([](const auto& transform)
  {
    using transform_t = std::decay_t<decltype(transform)>;
    if constexpr (std::is_same_v<transform_t, Transform_no_change>)
      return 1.0;

    else if constexpr (std::is_same_v<transform_t, Transform_rigid_change>)
      return transform.cosa;

    else if constexpr (std::is_same_v<transform_t, Transform_scale>)
      return transform.multiplier;

    else if constexpr (std::is_same_v<transform_t, Transform_extra_vertex>)
      return transform.vertex.x;

  }, transform);
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
// explicit deduction guide (not needed as of C++20)
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

double calc_with_overloaded(const Transform& transform)
{
  return std::visit(overloaded 
  {
    [](const Transform_no_change& transform)
    {
      return 1.0;
    },
    [](const Transform_rigid_change& transform)
    {
      return transform.cosa;
    },
    [](const Transform_scale& transform)
    {
      return transform.multiplier;
    }
    ,
    [](const Transform_extra_vertex& transform)
    {
      return transform.vertex.x;
    }
  }, transform);
}

double calc_with_do_overloaded(const DoTransform& transform)
{
  return std::visit(overloaded 
  {
    [](const auto& do_transform)
    {
      return do_transform();
    }
  }, transform);
}

std::vector<Transform> create_test_data(uint64_t size)
{
  std::vector<Transform> data;
  data.reserve(size*4);

  for (uint64_t i = 0; i != size; ++i)
  {
    if (i % 4 != 0)
      data.push_back(Transform_no_change{});

    const auto cosa = static_cast<double>(i)/size;
    const auto sina = sqrt(1.0-cosa*cosa);
    const auto x = 10*cosa;
    const auto y = static_cast<double>(i);

    if (i % 4 != 1)
      data.push_back(Transform_rigid_change{cosa, sina, {x,y}});

    if (i % 4 != 2)
      data.push_back(Transform_scale{2*cosa});

    if (i % 4 != 3)
      data.push_back(Transform_extra_vertex{i, {y,x}});
  }

  return data;
}

std::vector<Transform_union> create_union_test_data(uint64_t size)
{
  std::vector<Transform_union> data;
  data.reserve(size*4);

  for (uint64_t i = 0; i != size; ++i)
  {
    if (i % 4 != 0)
    {
      Transform_union transform;
      transform.type = Transform_union::Type::no_change;
      transform.no_change = Transform_no_change{};
      data.push_back(transform);
    }

    const auto cosa = static_cast<double>(i)/size;
    const auto sina = sqrt(1.0-cosa*cosa);
    const auto x = 10*cosa;
    const auto y = static_cast<double>(i);

    if (i % 4 != 1)
    {
      Transform_union transform;
      transform.type = Transform_union::Type::rigid_change;
      transform.rigid_change = Transform_rigid_change{cosa, sina, {x,y}};
      data.push_back(transform);
    }

    if (i % 4 != 2)
    {
      Transform_union transform;
      transform.type = Transform_union::Type::scale;
      transform.scale = Transform_scale{2*cosa};
      data.push_back(transform);
    }

    if (i % 4 != 3)
    {
      Transform_union transform;
      transform.type = Transform_union::Type::extra_vertex;
      transform.extra_vertex = Transform_extra_vertex{i, {y,x}};
      data.push_back(transform);
    }
  }

  return data;
}

std::vector<DoTransform> create_do_test_data(uint64_t size)
{
  std::vector<DoTransform> data;
  data.reserve(size*4);

  for (uint64_t i = 0; i != size; ++i)
  {
    if (i % 4 != 0)
    {
      data.push_back(transform_no_change());
    }

    const auto cosa = static_cast<double>(i)/size;
    const auto sina = sqrt(1.0-cosa*cosa);
    const auto x = 10*cosa;
    const auto y = static_cast<double>(i);

    if (i % 4 != 1)
    {
      data.push_back(transform_rigid_change(cosa, sina, {x, y}));
    }

    if (i % 4 != 2)
    {
      data.push_back(transform_scale(2*cosa));
    }

    if (i % 4 != 3)
    {
      data.push_back(transform_extra_vertex(i, {x, y}));
    }
  }

  return data;
}

template <typename TTestFnc, typename TTestFncArg>
double time_calc(TTestFnc fnc, size_t num_runs, const std::vector<TTestFncArg>& data)
{
  const auto start = std::chrono::system_clock::now();

  double total = 0;

  for (size_t run = 0; run != num_runs; ++run)
    for (const auto& transform: data)
      total += fnc(transform);

  const auto finish = std::chrono::system_clock::now();
  const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

  std::cout.precision(16);
  std::cout << total << "\t";

  return duration/1000.0;
}

int main()
{
  std::cout << "C++ enum calc\n";

  constexpr uint64_t data_size = 1000;
  const auto data = create_test_data(data_size);
  const auto union_data = create_union_test_data(data_size);
  const auto do_data = create_do_test_data(data_size);

  constexpr size_t num_runs = 200000;

  std::cout << "if        :\t" << time_calc(calc_with_if,         num_runs, data) << "s\n";
  std::cout << "switch    :\t" << time_calc(calc_with_switch,     num_runs, data) << "s\n";
  std::cout << "switch_if :\t" << time_calc(calc_with_switch_if,  num_runs, data) << "s\n";
  std::cout << "visit     :\t" << time_calc(calc_with_visit,      num_runs, data) << "s\n";
  std::cout << "overloaded:\t" << time_calc(calc_with_overloaded, num_runs, data) << "s\n";
  std::cout << "union     :\t" << time_calc(calc_with_union,      num_runs, union_data) << "s\n";
  std::cout << "lambdas   :\t" << time_calc(calc_with_do_overloaded,num_runs, do_data) << "s\n";
}
