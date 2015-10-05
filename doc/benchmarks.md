The benchmarks that are not promising because:

* wireshark: crash in Splitter: pugixml assert fail: (page->freed_size <= page->busy_size). This happens when parent.remove_child(decl_stmt_node). Don't know why.
* gzip: DECLARE macro
