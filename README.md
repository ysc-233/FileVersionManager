# FileVersionManager

Qt/C++ based local file version management tool.

## Status
Project skeleton initialized.

## Structure
        ┌──────────────┐
        │  FileWatcher │
        └──────┬───────┘
               │ fileChanged(path)
               ▼
        ┌──────────────┐
        │ VersionManager│
        └──────┬───────┘
               │
   ┌───────────┼────────────┐
   ▼           ▼            ▼
┌────────┐ ┌──────────┐ ┌────────────┐
│Hasher  │ │Metadata  │ │Storage     │
│SHA256  │ │Manager   │ │Manager     │
└────────┘ └────┬─────┘ └────┬───────┘
                 │            │
         metadata.json   .fvm/objects/

## UI data flow
metadata.json
     │
     ▼
MetadataManager
     │ QMap<file, versions>
     ▼
VersionManager
     │
     ▼
VersionTreeModel
     │
     ▼
QTreeView