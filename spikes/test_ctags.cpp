#include <readtags.h>
#include <iostream>

void print(tagEntry *entry) {
  std::cout<<"[Entry][name]"<<entry->name<<std::endl;
  std::cout<<"[Entry][file]"<<entry->file<<std::endl;
  std::cout<<"[Entry][Address][lineNumber]"<<entry->address.lineNumber<<std::endl;
  std::cout<<"[Entry][Address][Pattern]"<<entry->address.pattern<<std::endl;
  std::cout<<"[Entry][kind]"<<entry->kind<<std::endl;
  std::cout<<"[Entry][fileScope]"<<entry->fileScope<<std::endl;
  std::cout<<"[Entry][fields][count]"<<entry->fields.count<<std::endl;
  for (int i=0;i<entry->fields.count;i++) {
    std::cout<<"[Entry][fields][list][i][key]"<<entry->fields.list[i].key<<std::endl;
    std::cout<<"[Entry][fields][list][i][value]"<<entry->fields.list[i].value<<std::endl;
  }
}

int main() {
  tagFileInfo *info = (tagFileInfo*)malloc(sizeof(tagFileInfo));
  tagFile *file = tagsOpen ("/Users/hebi/tmp/tags", info);
  tagEntry *entry = (tagEntry*)malloc(sizeof(tagEntry));
  tagResult result = tagsFind(file, entry, "some_class", TAG_FULLMATCH);
  if (result == TagSuccess) {
    std::cout<<"success"<<std::endl;
    print(entry);
  } else {
    std::cout<<"failure"<<std::endl;
  }
  while(result == TagSuccess) {
    print(entry);
    std::cout<<"====="<<std::endl;
    result = tagsFindNext(file, entry);
  }

  return 0;
}
