#pragma once
#include <GLFW/glfw3.h>
#include <cstring>

#include "../basetools/EventSystem.h"

class IOManager
{
public:
    void Init(GLFWwindow* window)
    {
        _windowHandle = window;
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetMouseButtonCallback(window, MouseCallback);
        glfwSetCursorPosCallback(window, CursorCallback);
        glfwSetScrollCallback(window, ScrollCallback);

        // –≈√»—“–»–”≈Ã “≈ —“Œ¬€…  ŒÀ¡› 
        glfwSetCharCallback(window, CharCallback);
    }

    void Update()
    {
        std::memcpy(_keysPrevious, _keysCurrent, sizeof(_keysCurrent));
        _mousePrevious = _mouseCurrent;
        _scrollX = 0.0;
        _scrollY = 0.0;
    }

    // --- ‘”Õ ÷»» ƒÀﬂ œŒÀ‹«Œ¬¿“≈Àﬂ (ƒÀﬂ »√–€) ---

    //  À¿¬»¿“”–¿ » ÃŒƒ»‘» ¿“Œ–€
    static bool IsKeyDown(int key) { return _keysCurrent[key]; }
    static bool IsKeyPressed(int key) { return _keysCurrent[key] && !_keysPrevious[key]; }
    static bool IsKeyReleased(int key) { return !_keysCurrent[key] && _keysPrevious[key]; }
    //static bool IsCapsLockOn() { return _capsLock; }
    //static bool IsNumLockOn() { return _numLock; }

    // Ã€ÿ‹
    static bool IsMouseDown(int button) { return (_mouseCurrent & (1 << button)) != 0; }
    static bool IsMousePressed(int button) { return (_mouseCurrent & (1 << button)) && !(_mousePrevious & (1 << button)); }
    static bool IsMouseReleased(int button) { return !(_mouseCurrent & (1 << button)) && (_mousePrevious & (1 << button)); }

    // “–¿Õ—‘Œ–Ã Ã€ÿ»
    static void GetMousePos(double& x, double& y) { x = _mouseX; y = _mouseY; }
    static void GetScroll(double& x, double& y) { x = _scrollX; y = _scrollY; }

    // ”œ–¿¬À≈Õ»≈  ”–—Œ–OM
    static void ShowCursor() { glfwSetInputMode(_windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }
    static void HideCursor() { glfwSetInputMode(_windowHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN); }
    static void LockCursor() {
        glfwSetInputMode(_windowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported()) glfwSetInputMode(_windowHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    // “≈ —“Œ¬€… ¬¬Œƒ “≈ ”Ÿ≈√Œ  ¿ƒ–¿
    template<typename T, void(T::* Method)(unsigned int)>
    static size_t ConnectCharEvent(T* instance)
    {
        return _charEvent.connect(instance);
    }
    static void DisconnectCharEvent(size_t id)
    {
        _charEvent.disconnect(id);
    }

private:
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key < 0 || key > GLFW_KEY_LAST) return;
        if (action == GLFW_PRESS)  _keysCurrent[key] = true;
        else if (action == GLFW_RELEASE) _keysCurrent[key] = false;

        //_capsLock = (mods & GLFW_MOD_CAPS_LOCK) != 0;
        //_numLock = (mods & GLFW_MOD_NUM_LOCK) != 0;
    }
    static void MouseCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;
        if (action == GLFW_PRESS)  _mouseCurrent |= (1 << button);
        else if (action == GLFW_RELEASE) _mouseCurrent &= ~(1 << button);
    }
    static void CharCallback(GLFWwindow* window, unsigned int codepoint)
    {
        _charEvent.emit(codepoint);
    }

    static void CursorCallback(GLFWwindow* window, double xpos, double ypos) { _mouseX = xpos; _mouseY = ypos; }
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) { _scrollX = xoffset; _scrollY = yoffset; }



private:
    inline static Event<unsigned int> _charEvent;
    inline static GLFWwindow* _windowHandle = nullptr;
    inline static double _mouseX = 0.0, _mouseY = 0.0;
    inline static double _scrollX = 0.0, _scrollY = 0.0;
    inline static uint8_t _mouseCurrent = 0;
    inline static uint8_t _mousePrevious = 0;
    inline static bool _keysCurrent[GLFW_KEY_LAST + 1] = { false };
    inline static bool _keysPrevious[GLFW_KEY_LAST + 1] = { false };
    //inline static bool _capsLock = false;
    //inline static bool _numLock = false;
};