#include "Toolbar.h"
#include "MainApplication.h"
#include "geometry/Geometry.h"
#include "imgui_internal.h"
#include "tools/ExportAssistant.h"

namespace pepr3d {

void Toolbar::draw() {
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoNav;

    glm::ivec2 size(mApplication.getWindowSize().x + 2.0f - mApplication.getSidePane().getWidth(), mHeight);

    ImGui::SetNextWindowPos(glm::ivec2(-1, 0));
    ImGui::SetNextWindowSize(size);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ci::ColorA::hex(0xFFFFFF));
    ImGui::PushStyleColor(ImGuiCol_Border, ci::ColorA::hex(0xE5E5E5));
    ImGui::PushStyleColor(ImGuiCol_Text, ci::ColorA::hex(0x1C2A35));
    ImGui::PushStyleColor(ImGuiCol_Button, ci::ColorA::zero());
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ci::ColorA::hex(0xCFD5DA));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ci::ColorA::hex(0xA3B2BF));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, glm::vec2(0.5f, 0.5f));

    ImGui::Begin("##toolbar", nullptr, window_flags);

    drawFileDropDown();
    ImGui::SameLine(0.0f, 0.0f);
    drawSeparator();
    ImGui::SameLine(0.0f, 0.0f);
    drawUndoRedo();
    ImGui::SameLine(0.0f, 0.0f);
    drawSeparator();
    drawToolButtons();

    ImGui::End();

    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(6);
}

void Toolbar::drawSeparator() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    glm::ivec2 cursorPos = ImGui::GetCursorScreenPos();
    drawList->AddLine(cursorPos, cursorPos + glm::ivec2(0, mHeight), ImColor(ci::ColorA::hex(0xE5E5E5)));
    ImGui::SetCursorScreenPos(cursorPos + glm::ivec2(1, 0));
}

void Toolbar::drawFileDropDown() {
    static const char* filePopupId = "##filepopup";
    ButtonProperties props;
    props.label = ICON_MD_FOLDER_OPEN;
    props.isDropDown = true;
    props.isToggled = ImGui::IsPopupOpen(filePopupId);
    glm::vec2 buttonPos = ImGui::GetCursorScreenPos();
    ImGui::PushFont(mApplication.getFontStorage().getRegularIconFont());
    drawButton(props, [&]() {
        props.isToggled = !props.isToggled;
        if(props.isToggled) {
            ImGui::OpenPopup(filePopupId);
        }
    });
    ImGui::PopFont();
    mApplication.drawTooltipOnHover("File", "", "Open, save, import, and export a model.", "",
                                    buttonPos + glm::vec2(0.0f, mHeight + 6.0f));

    if(props.isToggled) {
        ImGui::SetNextWindowPos(glm::ivec2(0, mHeight - 1));
        ImGui::SetNextWindowSize(glm::ivec2(175, 300));
        if(ImGui::BeginPopup(filePopupId)) {
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, glm::vec2(0.5f, 0.5f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::ivec2(0, 0));
            if(ImGui::Button("Open", glm::ivec2(175, 50))) {
                ImGui::CloseCurrentPopup();
                mApplication.showImportDialog(mApplication.supportedOpenExtensions);
            }
            if(ImGui::Button("Save", glm::ivec2(175, 50))) {
                ImGui::CloseCurrentPopup();
                mApplication.saveProject();
            }
            if(ImGui::Button("Save as...", glm::ivec2(175, 50))) {
                ImGui::CloseCurrentPopup();
                mApplication.saveProjectAs();
            }
            if(ImGui::Button("Import", glm::ivec2(175, 50))) {
                ImGui::CloseCurrentPopup();
                mApplication.showImportDialog(mApplication.supportedImportExtensions);
            }
            if(ImGui::Button("Export", glm::ivec2(175, 50))) {
                ImGui::CloseCurrentPopup();
                mApplication.setCurrentTool<ExportAssistant>();
            }
            if(ImGui::Button("Exit", glm::ivec2(175, 50))) {
                mApplication.quit();
            }
            ImGui::PopStyleVar(2);
            ImGui::EndPopup();
        }
    }
}

void Toolbar::drawUndoRedo() {
    CommandManager<Geometry>* const commandManager = mApplication.getCommandManager();
    ButtonProperties props;
    props.label = ICON_MD_UNDO;
    props.isEnabled = commandManager && commandManager->canUndo();
    glm::vec2 buttonPos = ImGui::GetCursorScreenPos();
    ImGui::PushFont(mApplication.getFontStorage().getRegularIconFont());
    drawButton(props, [this]() {
        mApplication.enqueueSlowOperation([this]() { mApplication.getCommandManager()->undo(); }, []() {});
    });
    const auto undoOptionalHotkey = mApplication.getHotkeys().findHotkey(HotkeyAction::Undo);
    mApplication.drawTooltipOnHover("Undo", undoOptionalHotkey ? undoOptionalHotkey->getString() : "",
                                    "Undo last action.", props.isEnabled ? "" : "No action to undo.",
                                    buttonPos + glm::vec2(0.0f, mHeight + 6.0f));
    ImGui::SameLine(0.f, 0.f);
    props.label = ICON_MD_REDO;
    props.isEnabled = commandManager && commandManager->canRedo();
    buttonPos = ImGui::GetCursorScreenPos();
    drawButton(props, [this]() {
        mApplication.enqueueSlowOperation([this]() { mApplication.getCommandManager()->redo(); }, []() {});
    });
    ImGui::PopFont();
    const auto redoOptionalHotkey = mApplication.getHotkeys().findHotkey(HotkeyAction::Redo);
    mApplication.drawTooltipOnHover("Redo", redoOptionalHotkey ? redoOptionalHotkey->getString() : "",
                                    "Redo last action.", props.isEnabled ? "" : "No action to redo.",
                                    buttonPos + glm::vec2(0.0f, mHeight + 6.0f));
}

void Toolbar::drawToolButtons() {
    std::size_t index = 0;
    for(auto toolit = mApplication.getToolsBegin(); toolit != mApplication.getToolsEnd(); ++toolit, ++index) {
        ImGui::SameLine(0.0f, 0.0f);
        const glm::vec2 buttonPos = ImGui::GetCursorScreenPos();
        drawToolButton(toolit);
        const auto toolOptionalHotkey = (*toolit)->getHotkey(mApplication.getHotkeys());
        mApplication.drawTooltipOnHover((*toolit)->getName(), toolOptionalHotkey ? toolOptionalHotkey->getString() : "",
                                        (*toolit)->getDescription(),
                                        (*toolit)->isEnabled() ? "" : "Not available for this geometry.",
                                        buttonPos + glm::vec2(0.0f, mHeight + 6.0f));
        if(index == 5 || index == 8) {
            ImGui::SameLine(0.0f, 0.0f);
            drawSeparator();
        }
    }
}

void Toolbar::drawToolButton(ToolsVector::iterator tool) {
    ButtonProperties props;
    props.label = (*tool)->getIcon();
    props.isToggled = (tool == mApplication.getCurrentToolIterator());
    props.isEnabled = (*tool)->isEnabled();
    ImGui::PushFont(mApplication.getFontStorage().getRegularIconFont());
    drawButton(props, [&]() {
        props.isToggled = true;
        mApplication.setCurrentToolIterator(tool);
    });
    ImGui::PopFont();
}

}  // namespace pepr3d