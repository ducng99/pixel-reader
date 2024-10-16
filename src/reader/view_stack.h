#ifndef VIEW_STACK_H_
#define VIEW_STACK_H_

#include "./view.h"

#include <memory>
#include <vector>

class ViewStack: public View
{
    std::vector<std::shared_ptr<View>> views;
    std::weak_ptr<View> last_top_view;
public:
    void push(std::shared_ptr<View> view);
    virtual ~ViewStack();

    bool render(SDL_Surface *dest, bool force_render) override;
    bool is_done() override;

    void on_keypress(SDL_Scancode key) override;
    void on_keyheld(SDL_Scancode key, uint32_t hold_time_ms) override;

    // Pop views that report as done. Return true if focus changed.
    bool pop_completed_views();
    // Pop all views.
    void shutdown();

    std::shared_ptr<View> top_view() const;
};

#endif
