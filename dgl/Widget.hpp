/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2021 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef DGL_WIDGET_HPP_INCLUDED
#define DGL_WIDGET_HPP_INCLUDED

#include "Geometry.hpp"

START_NAMESPACE_DGL

// --------------------------------------------------------------------------------------------------------------------
// Forward class names

class Application;
class SubWidget;
class TopLevelWidget;
class Window;

// --------------------------------------------------------------------------------------------------------------------

/**
   Base DGL Widget class.

   This is the base Widget class, from which all widgets are built.

   All widgets have a parent widget where they'll be drawn, this can be the top-level widget or a group widget.
   This parent is never changed during a widget's lifetime.

   Widgets receive events in relative coordinates. (0, 0) means its top-left position.

   The top-level widget will draw subwidgets in the order they are constructed.
   Early subwidgets are drawn first, at the bottom, then newer ones on top.
   Events are sent in the inverse order so that the top-most widgets get
   a chance to catch the event and stop its propagation.

   All widget event callbacks do nothing by default and onDisplay MUST be reimplemented by subclasses.

   @note It is not possible to subclass this Widget class directly, you must use SubWidget or TopLevelWidget instead.
 */
class Widget
{
public:
   /**
      Base event data.
      These are the fields present on all Widget events.

      @a mod  Currently active keyboard modifiers, @see Modifier.
      @a mod  Event flags, @see Flag.
      @a time Event timestamp (if any).
    */
    struct BaseEvent {
        uint mod;
        uint flags;
        uint time;

        /** Constuctor */
        BaseEvent() noexcept : mod(0x0), flags(0x0), time(0) {}
        /** Destuctor */
        virtual ~BaseEvent() noexcept {}
    };

   /**
      Keyboard event.

      This event represents low-level key presses and releases.
      This can be used for "direct" keyboard handing like key bindings, but must not be interpreted as text input.

      Keys are represented portably as Unicode code points, using the "natural" code point for the key.
      The @a key field is the code for the pressed key, without any modifiers applied.
      For example, a press or release of the 'A' key will have `key` 97 ('a')
      regardless of whether shift or control are being held.

      Alternatively, the raw @a keycode can be used to work directly with physical keys,
      but note that this value is not portable and differs between platforms and hardware.

      @a press   True if the key was pressed, false if released.
      @a key     Unicode point of the key pressed.
      @a keycode Raw keycode.
      @see onKeyboard
    */
    struct KeyboardEvent : BaseEvent {
        bool press;
        uint key;
        uint keycode;

        /** Constuctor */
        KeyboardEvent() noexcept
            : BaseEvent(),
              press(false),
              key(0),
              keycode(0) {}
    };

   /**
      Special keyboard event.

      This event allows the use of keys that do not have unicode points.
      Note that some are non-printable keys.

      @a press True if the key was pressed, false if released.
      @a key   The key pressed.
      @see onSpecial
    */
    struct SpecialEvent : BaseEvent {
        bool press;
        Key  key;

        /** Constuctor */
        SpecialEvent() noexcept
            : BaseEvent(),
              press(false),
              key(Key(0)) {}
    };

   /**
      Character input event.

      This event represents text input, usually as the result of a key press.
      The text is given both as a Unicode character code and a UTF-8 string.

      Note that this event is generated by the platform's input system,
      so there is not necessarily a direct correspondence between text events and physical key presses.
      For example, with some input methods a sequence of several key presses will generate a single character.

      @a keycode   Raw key code.
      @a character Unicode character code.
      @a string    UTF-8 string.
      @see onCharacterInput
    */
    struct CharacterInputEvent : BaseEvent {
        uint keycode;
        uint character;
        char string[8];

        /** Constuctor */
        CharacterInputEvent() noexcept
            : BaseEvent(),
              keycode(0),
              character(0),
              string{'\0','\0','\0','\0','\0','\0','\0','\0'} {}
    };

   /**
      Mouse press or release event.

      @a button The button number starting from 1 (1 = left, 2 = middle, 3 = right).
      @a press  True if the button was pressed, false if released.
      @a pos    The widget-relative coordinates of the pointer.
      @see onMouse
    */
    struct MouseEvent : BaseEvent {
        uint button;
        bool press;
        Point<double> pos;

        /** Constuctor */
        MouseEvent() noexcept
            : BaseEvent(),
              button(0),
              press(false),
              pos(0.0, 0.0) {}
    };

   /**
      Mouse motion event.

      @a pos The widget-relative coordinates of the pointer.
      @see onMotion
    */
    struct MotionEvent : BaseEvent {
        Point<double> pos;

        /** Constuctor */
        MotionEvent() noexcept
            : BaseEvent(),
              pos(0.0, 0.0) {}
    };

   /**
      Mouse scroll event.

      The scroll distance is expressed in "lines",
      an arbitrary unit that corresponds to a single tick of a detented mouse wheel.
      For example, `delta.y` = 1.0 scrolls 1 line up.
      Some systems and devices support finer resolution and/or higher values for fast scrolls,
      so programs should handle any value gracefully.

      @a pos       The widget-relative coordinates of the pointer.
      @a delta     The scroll distance.
      @a direction The direction of the scroll or "smooth".
      @see onScroll
    */
    struct ScrollEvent : BaseEvent {
        Point<double> pos;
        Point<double> delta;
        ScrollDirection direction;

        /** Constuctor */
        ScrollEvent() noexcept
            : BaseEvent(),
              pos(0.0, 0.0),
              delta(0.0, 0.0),
              direction(kScrollSmooth) {}
    };

   /**
      Resize event.
      @a size    The new widget size.
      @a oldSize The previous size, may be null.
      @see onResize
    */
    struct ResizeEvent {
        Size<uint> size;
        Size<uint> oldSize;

        /** Constuctor */
        ResizeEvent() noexcept
            : size(0, 0),
              oldSize(0, 0) {}
    };

   /**
      Widget position changed event.
      @a pos    The new absolute position of the widget.
      @a oldPos The previous absolute position of the widget.
      @see onPositionChanged
    */
    struct PositionChangedEvent {
        Point<int> pos;
        Point<int> oldPos;

        /** Constuctor */
        PositionChangedEvent() noexcept
            : pos(0, 0),
              oldPos(0, 0) {}
    };

private:
   /**
      Private constructor, reserved for TopLevelWidget class.
    */
    explicit Widget(TopLevelWidget* topLevelWidget);

   /**
      Private constructor, reserved for SubWidget class.
    */
    explicit Widget(Widget* widgetToGroupTo);

public:
   /**
      Destructor.
    */
    virtual ~Widget();

   /**
      Check if this widget is visible within its parent window.
      Invisible widgets do not receive events except resize.
    */
    bool isVisible() const noexcept;

   /**
      Set widget visible (or not) according to @a visible.
    */
    void setVisible(bool visible);

   /**
      Show widget.
      This is the same as calling setVisible(true).
    */
    void show();

   /**
      Hide widget.
      This is the same as calling setVisible(false).
    */
    void hide();

   /**
      Get width.
    */
    uint getWidth() const noexcept;

   /**
      Get height.
    */
    uint getHeight() const noexcept;

   /**
      Get size.
    */
    const Size<uint> getSize() const noexcept;

   /**
      Set width.
    */
    void setWidth(uint width) noexcept;

   /**
      Set height.
    */
    void setHeight(uint height) noexcept;

   /**
      Set size using @a width and @a height values.
    */
    void setSize(uint width, uint height) noexcept;

   /**
      Set size.
    */
    void setSize(const Size<uint>& size) noexcept;

   /**
      Get the Id associated with this widget.
      @see setId
    */
    uint getId() const noexcept;

   /**
      Set an Id to be associated with this widget.
      @see getId
    */
    void setId(uint id) noexcept;

   /**
      Get the application associated with this widget's window.
      This is the same as calling `getTopLevelWidget()->getApp()`.
    */
    Application& getApp() const noexcept;

   /**
      Get the window associated with this widget.
      This is the same as calling `getTopLevelWidget()->getWindow()`.
    */
    Window& getWindow() const noexcept;

   /**
      Get the graphics context associated with this widget's window.
      GraphicsContext is an empty struct and needs to be casted into a different type in order to be usable,
      for example GraphicsContext.
      @see CairoSubWidget, CairoTopLevelWidget
    */
    const GraphicsContext& getGraphicsContext() const noexcept;

   /**
      Get top-level widget, as passed directly in the constructor
      or going up the chain of group widgets until it finds the top-level one.
    */
    TopLevelWidget* getTopLevelWidget() const noexcept;

   /**
      Request repaint of this widget's area to the window this widget belongs to.
      On the raw Widget class this function does nothing.
    */
    virtual void repaint() noexcept;

    DISTRHO_DEPRECATED_BY("getApp()")
    Application& getParentApp() const noexcept { return getApp(); }

    DISTRHO_DEPRECATED_BY("getWindow()")
    Window& getParentWindow() const noexcept { return getWindow(); }

protected:
   /**
      A function called to draw the widget contents.
    */
    virtual void onDisplay() = 0;

   /**
      A function called when a key is pressed or released.
      @return True to stop event propagation, false otherwise.
    */
    virtual bool onKeyboard(const KeyboardEvent&);

   /**
      A function called when a special key is pressed or released.
      @return True to stop event propagation, false otherwise.
    */
    virtual bool onSpecial(const SpecialEvent&);

   /**
      A function called when an UTF-8 character is received.
      @return True to stop event propagation, false otherwise.
    */
    virtual bool onCharacterInput(const CharacterInputEvent&);

   /**
      A function called when a mouse button is pressed or released.
      @return True to stop event propagation, false otherwise.
    */
    virtual bool onMouse(const MouseEvent&);

   /**
      A function called when the pointer moves.
      @return True to stop event propagation, false otherwise.
    */
    virtual bool onMotion(const MotionEvent&);

   /**
      A function called on scrolling (e.g. mouse wheel or track pad).
      @return True to stop event propagation, false otherwise.
    */
    virtual bool onScroll(const ScrollEvent&);

   /**
      A function called when the widget is resized.
    */
    virtual void onResize(const ResizeEvent&);

private:
    struct PrivateData;
    PrivateData* const pData;
    friend class SubWidget;
    friend class TopLevelWidget;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Widget)
};

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DGL

#endif // DGL_WIDGET_HPP_INCLUDED
