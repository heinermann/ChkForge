# ChkForge
Starcraft map editor backed by Qt, ChkDraft, and OpenBW. Inspired by Starforge and SCMDraft.

Some code snippets from Starforge are used (i.e. terrain).

## Libraries & Technologies Used
- [Qt](https://www.qt.io/) for GUI and other utilities.
- [Qt-Advanced-Docking-System](https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System) for better docking UI.
- [MappingCoreLib](https://github.com/jjf28/Chkdraft/tree/master/MappingCoreLib) (ChkDraft) and its dependencies for map/chk management.
- [OpenBW](https://github.com/OpenBW/openbw) for rendering and simulation.
- [StormLib](https://github.com/ladislav-zezula/StormLib) for MPQ management.
- [CascLib](https://github.com/ladislav-zezula/CascLib) for Casc loading.
- [oxygen-icons-png](https://github.com/pasnox/oxygen-icons-png) for icons.

## Development
### Developer Setup
1. Install [Visual Studio 2019: Community Edition](https://visualstudio.microsoft.com/).
2. Install [Qt](https://www.qt.io/download), under *Downloads for open source users*.
3. Install [Qt Visual Studio Tools](https://marketplace.visualstudio.com/items?itemName=TheQtCompany.QtVisualStudioTools2019) in Visual Studio (the application) under *Extensions* -> *Manage Extensions*.
4. `git clone --recurse-submodules git@github.com:heinermann/ChkForge.git`
5. Open **Chkforge.sln** with Visual Studio 2019.

### Some Class Info
- `MapContext` is the class that bridges ChkDraft and OpenBW between each other, and is used to programmatically interact with the map and openbw "game".
- `MapView` is the window that *shows* a `MapContext`. There can be more than one `MapView` with the same map context.
- `MainWindow` is self explanatory.
- Different UI components are separated as follows:
  - Tool Windows (minimap, item tree, etc)
  - Property Dialogs (unit, sprite, location properties w/ enter key)
  - Other Dialogs (New Map, Unit Settings, etc)
- ChkDraft classes are used for chk manipulation, but in general we should only be doing things through `MapContext` to also reflect those changes in OpenBW.
- OpenBW classes are just a mess of header files.

### Things to Keep in Mind
- Use `tr()` for strings to create translatable strings in-code. ([more info](https://doc.qt.io/qt-5/i18n-source-translation.html#using-tr-for-all-literal-text))
- Only interact with maps through `MapContext`, and not directly through ChkDraft/OpenBW.
- Avoid globals.
- Try to keep the boundaries between ChkForge and OpenBW separate (as much as possible, i.e. don't start using QString in OpenBW).

### Libraries to Investigate
- [QSimpleUpdater](https://github.com/alex-spataru/QSimpleUpdater) for automatic updates?
- Possibly some sound library for ogg, but Qt might support it already, though Qt doesn't support directional sound or sound mixing.
