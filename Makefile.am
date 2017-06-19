ACLOCAL_AMFLAGS=-I m4
SUBDIRS = inc

#The logging library
lib_LTLIBRARIES = libopx_common.la

libopx_common_la_SOURCES = \
src/std_ip_utils.c    src/std_socket_service.cpp  \
src/std_bit_masks.c         src/std_llist.c       src/std_socket_tools.cpp \
src/std_cmd_redir.c         src/std_mac_utils.c   src/std_string_utils.cpp \
src/std_config_file.cpp     src/std_mergesort.c   src/std_system.c  \
src/std_config_node.cpp       src/std_mutex_lock.c  src/std_thread_pool.cpp \
src/std_directory.cpp       src/std_radical.c     src/std_thread_tools.c \
src/std_event_service.cpp   src/std_radix.c       src/std_time_tools.c \
src/std_event_utils.cpp     src/std_rbtree.c      src/std_user_perm.cpp \
src/std_file_utils.c        src/std_select.c      \
src/std_int_mapping_util.c  src/std_shlib.c

libopx_common_la_CPPFLAGS = -I$(top_srcdir)/inc/opx -I$(includedir)/libxml2 -I$(includedir)/opx
libopx_common_la_CXXFLAGS = -std=c++11
libopx_common_la_LDFLAGS = -shared -version-info 1:1:0
libopx_common_la_LIBADD = -lopx_logging -lxml2 -lpthread -lrt
