#ifndef INPUT_H
#define INPUT_H

#include <memory>
#include <string_view>

class InputImpl;
namespace qls {

class Input final {
public:
  Input();
  ~Input();

  void init();

  bool input(std::string_view command);

private:
  std::unique_ptr<InputImpl> m_impl;
};

} // namespace qls

#endif // !INPUT_H
