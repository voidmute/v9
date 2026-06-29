<p align="center">
  <img src="assets/logo-transparent.png" alt="v9" width="160">
</p>

<p align="center">
  <img src="https://api.iconify.design/feather/zap.svg?color=%23ff6a1a&height=16" width="16" alt="" align="absmiddle">
  <strong>Cheat Menu для Meccha Chameleon</strong> - визуалы, FOV, список игроков и телепорт. Всё в одном меню, быстро включается и настраивается.
  <img src="https://api.iconify.design/feather/zap.svg?color=%23ff6a1a&height=16" width="16" alt="" align="absmiddle">
</p>

<p align="center">
  <a href="https://github.com/voidmute/v9/releases/latest"><img src="https://img.shields.io/github/v/release/voidmute/v9?style=flat-square&label=latest&labelColor=6b7280&color=ff6a1a" alt="latest v1.0.0"></a>
  &nbsp;
  <a href="https://github.com/voidmute/v9?tab=MIT-1-ov-file"><img src="https://img.shields.io/badge/license-MIT-ff6a1a?style=flat-square" alt="License: MIT"></a>
  &nbsp;
  <a href="https://www.microsoft.com/windows/get-windows"><img src="https://img.shields.io/badge/platform-Windows%20x64-ff6a1a?style=flat-square" alt="Platform"></a>
</p>

---

## <img src="https://api.iconify.design/feather/layers.svg?color=%23ff6a1a&height=20" width="20" alt="" align="absmiddle"> Возможности

| | Раздел | Описание |
|:---:|:------:|----------|
| <img src="https://api.iconify.design/feather/eye.svg?color=%23ff6a1a&height=18" width="18" alt=""> | **Визуал** | Рамка, скелет, линии, имена, роли, дистанция, фильтр врагов |
| <img src="https://api.iconify.design/feather/aperture.svg?color=%23ff6a1a&height=18" width="18" alt=""> | **Камера** | Кастомный FOV |
| <img src="https://api.iconify.design/feather/users.svg?color=%23ff6a1a&height=18" width="18" alt=""> | **Игроки** | Список игроков и телепорт |
| <img src="https://api.iconify.design/feather/droplet.svg?color=%23ff6a1a&height=18" width="18" alt=""> | **Цвета** | Палитра визуалов + сохранение настроек |

<img src="https://api.iconify.design/feather/check-circle.svg?color=%23ff6a1a&height=16" width="16" alt="" align="absmiddle"> Меню открывается за секунду, настройки сохраняются сами - не нужно лезть в файлы.

---

## <img src="https://api.iconify.design/feather/rocket.svg?color=%23ff6a1a&height=20" width="20" alt="" align="absmiddle"> Быстрый старт

### <img src="https://api.iconify.design/feather/download.svg?color=%23ff6a1a&height=18" width="18" alt="" align="absmiddle"> 1. Скачайте релиз

Перейдите в [последний релиз](https://github.com/voidmute/v9/releases/latest) и скачайте **`v9-esp-win64.zip`**.

### <img src="https://api.iconify.design/feather/folder.svg?color=%23ff6a1a&height=18" width="18" alt="" align="absmiddle"> 2. Распакуйте в одну папку

```
v9/
├── v9.dll
├── v9injector.exe
├── inject-v9-esp.bat
└── inject-v9-esp.ps1
```

### <img src="https://api.iconify.design/feather/monitor.svg?color=%23ff6a1a&height=18" width="18" alt="" align="absmiddle"> 3. Запустите игру

Процесс: **`Meccha Chameleon`**

### <img src="https://api.iconify.design/feather/target.svg?color=%23ff6a1a&height=18" width="18" alt="" align="absmiddle"> 4. Инжект

Дважды кликните **`inject-v9-esp.bat`** (при ошибке - от имени администратора).

Или в PowerShell:

```powershell
cd путь\к\папке
.\v9injector.exe PenguinHotel-Win64-Shipping.exe ".\v9.dll"
```

---

## <img src="https://api.iconify.design/feather/command.svg?color=%23ff6a1a&height=20" width="20" alt="" align="absmiddle"> Горячие клавиши

| Клавиша | Действие |
|---------|----------|
| **INSERT** <img src="https://api.iconify.design/feather/toggle-right.svg?color=%23ff6a1a&height=14" width="14" alt="" align="absmiddle"> | Открыть / закрыть меню |
| **END** <img src="https://api.iconify.design/feather/log-out.svg?color=%23ff6a1a&height=14" width="14" alt="" align="absmiddle"> | Выгрузить Cheat Menu из игры |

<img src="https://api.iconify.design/feather/save.svg?color=%23ff6a1a&height=16" width="16" alt="" align="absmiddle"> Настройки сохраняются в `C:\v9\settings.ini` (папка создаётся автоматически).

---

## <img src="https://api.iconify.design/feather/package.svg?color=%23ff6a1a&height=20" width="20" alt="" align="absmiddle"> Сборка из исходников

<img src="https://api.iconify.design/feather/cpu.svg?color=%23ff6a1a&height=16" width="16" alt="" align="absmiddle"> **Требования:** Windows 10/11 x64, Visual Studio 2022 Build Tools (MSVC v143), Windows SDK.

```powershell
cd v9
.\build.bat
```

Готовые файлы появятся в папке **`built/`**.

Полная сборка (Cheat Menu + инжектор + camouflage):

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_all.ps1
```

---

## <img src="https://api.iconify.design/feather/git-branch.svg?color=%23ff6a1a&height=20" width="20" alt="" align="absmiddle"> Структура репозитория

```
v9/
├── assets/          # Логотип (logo-transparent.png)
├── v9/              # Исходники Cheat Menu (DLL)
├── runtime/         # Инжектор, bridge, camouflage
├── scripts/         # Скрипты сборки
├── built/           # Собранные бинарники (после build)
├── build.bat
└── README.md
```

---

## <img src="https://api.iconify.design/feather/alert-triangle.svg?color=%23ff6a1a&height=20" width="20" alt="" align="absmiddle"> Важно

- <img src="https://api.iconify.design/feather/shield.svg?color=%23ff6a1a&height=14" width="14" alt="" align="absmiddle"> Используйте **только на свой страх и риск**. Автор не несёт ответственности за баны или сбои.
- <img src="https://api.iconify.design/feather/slash.svg?color=%23ff6a1a&height=14" width="14" alt="" align="absmiddle"> Не запускайте **Cheat Menu и camouflage одновременно** - возможен конфликт D3D12-хуков.
- <img src="https://api.iconify.design/feather/book-open.svg?color=%23ff6a1a&height=14" width="14" alt="" align="absmiddle"> Репозиторий предназначен для образовательных целей и личного использования.

---

## <img src="https://api.iconify.design/feather/file-text.svg?color=%23ff6a1a&height=20" width="20" alt="" align="absmiddle"> Лицензия

[MIT](https://github.com/voidmute/v9?tab=MIT-1-ov-file) © 2026 [voidmute](https://github.com/voidmute)
