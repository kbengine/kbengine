headerdir = $(prefix)/include/libmongoc-@MONGOC_API_VERSION@
header_DATA = \
	$(INST_H_FILES) \
	$(MONGOC_DEF_FILES) \
	src/mongoc/mongoc-config.h

MONGOC_DEF_FILES = \
	src/mongoc/op-delete.def \
	src/mongoc/op-get-more.def \
	src/mongoc/op-header.def \
	src/mongoc/op-insert.def \
	src/mongoc/op-kill-cursors.def \
	src/mongoc/op-msg.def \
	src/mongoc/op-query.def \
	src/mongoc/op-reply.def \
	src/mongoc/op-update.def \
	src/mongoc/mongoc-counters.defs

INST_H_FILES = \
	src/mongoc/mongoc.h \
	src/mongoc/mongoc-array-private.h \
	src/mongoc/mongoc-async-private.h \
	src/mongoc/mongoc-async-cmd-private.h \
	src/mongoc/mongoc-b64-private.h \
	src/mongoc/mongoc-buffer-private.h \
	src/mongoc/mongoc-bulk-operation-private.h \
	src/mongoc/mongoc-bulk-operation.h \
	src/mongoc/mongoc-client-pool.h \
	src/mongoc/mongoc-client-pool-private.h \
	src/mongoc/mongoc-client-private.h \
	src/mongoc/mongoc-client.h \
	src/mongoc/mongoc-cluster-private.h \
	src/mongoc/mongoc-collection-private.h \
	src/mongoc/mongoc-collection.h \
	src/mongoc/mongoc-counters-private.h \
	src/mongoc/mongoc-cursor-array-private.h \
	src/mongoc/mongoc-cursor-cursorid-private.h \
	src/mongoc/mongoc-cursor-transform-private.h \
	src/mongoc/mongoc-cursor-private.h \
	src/mongoc/mongoc-cursor.h \
	src/mongoc/mongoc-database-private.h \
	src/mongoc/mongoc-database.h \
	src/mongoc/mongoc-errno-private.h \
	src/mongoc/mongoc-error.h \
	src/mongoc/mongoc-find-and-modify-private.h \
	src/mongoc/mongoc-find-and-modify.h \
	src/mongoc/mongoc-flags.h \
	src/mongoc/mongoc-gridfs-file-list-private.h \
	src/mongoc/mongoc-gridfs-file-list.h \
	src/mongoc/mongoc-gridfs-file-page-private.h \
	src/mongoc/mongoc-gridfs-file-page.h \
	src/mongoc/mongoc-gridfs-file-private.h \
	src/mongoc/mongoc-gridfs-file.h \
	src/mongoc/mongoc-gridfs-private.h \
	src/mongoc/mongoc-gridfs.h \
	src/mongoc/mongoc-host-list-private.h \
	src/mongoc/mongoc-host-list.h \
	src/mongoc/mongoc-index.h \
	src/mongoc/mongoc-init.h \
	src/mongoc/mongoc-iovec.h \
	src/mongoc/mongoc-list-private.h \
	src/mongoc/mongoc-log.h \
	src/mongoc/mongoc-log-private.h \
	src/mongoc/mongoc-matcher-op-private.h \
	src/mongoc/mongoc-matcher-private.h \
	src/mongoc/mongoc-matcher.h \
	src/mongoc/mongoc-memcmp-private.h \
	src/mongoc/mongoc-opcode.h \
	src/mongoc/mongoc-opcode-private.h \
	src/mongoc/mongoc-queue-private.h \
	src/mongoc/mongoc-read-concern-private.h \
	src/mongoc/mongoc-read-concern.h \
	src/mongoc/mongoc-read-prefs-private.h \
	src/mongoc/mongoc-read-prefs.h \
	src/mongoc/mongoc-rpc-private.h \
	src/mongoc/mongoc-sasl-private.h \
	src/mongoc/mongoc-scram-private.h \
	src/mongoc/mongoc-server-description.h \
	src/mongoc/mongoc-server-description-private.h \
	src/mongoc/mongoc-server-stream-private.h \
	src/mongoc/mongoc-set-private.h \
	src/mongoc/mongoc-socket.h \
	src/mongoc/mongoc-socket-private.h \
	src/mongoc/mongoc-ssl-private.h \
	src/mongoc/mongoc-stream-buffered.h \
	src/mongoc/mongoc-stream-file.h \
	src/mongoc/mongoc-stream-gridfs.h \
	src/mongoc/mongoc-stream-private.h \
	src/mongoc/mongoc-stream-socket.h \
	src/mongoc/mongoc-stream.h \
	src/mongoc/mongoc-thread-private.h \
	src/mongoc/mongoc-topology-description-private.h \
	src/mongoc/mongoc-topology-private.h \
	src/mongoc/mongoc-topology-scanner-private.h \
	src/mongoc/mongoc-trace.h \
	src/mongoc/mongoc-uri.h \
	src/mongoc/mongoc-uri-private.h \
	src/mongoc/mongoc-util-private.h \
	src/mongoc/mongoc-version.h \
	src/mongoc/mongoc-version-functions.h \
	src/mongoc/mongoc-write-command-private.h \
	src/mongoc/mongoc-write-concern-private.h \
	src/mongoc/mongoc-write-concern.h \
	src/mongoc/utlist.h

if ENABLE_SSL
INST_H_FILES += \
	src/mongoc/mongoc-rand.h \
	src/mongoc/mongoc-rand-private.h \
	src/mongoc/mongoc-stream-tls.h \
	src/mongoc/mongoc-ssl.h
endif

MONGOC_SOURCES_SHARED += \
	$(INST_H_FILES) \
	src/mongoc/mongoc-array.c \
	src/mongoc/mongoc-async.c \
	src/mongoc/mongoc-async-cmd.c \
	src/mongoc/mongoc-buffer.c \
	src/mongoc/mongoc-bulk-operation.c \
	src/mongoc/mongoc-b64.c \
	src/mongoc/mongoc-client.c \
	src/mongoc/mongoc-client-pool.c \
	src/mongoc/mongoc-cluster.c \
	src/mongoc/mongoc-collection.c \
	src/mongoc/mongoc-counters.c \
	src/mongoc/mongoc-cursor.c \
	src/mongoc/mongoc-cursor-array.c \
	src/mongoc/mongoc-cursor-cursorid.c \
	src/mongoc/mongoc-cursor-transform.c \
	src/mongoc/mongoc-database.c \
	src/mongoc/mongoc-find-and-modify.c \
	src/mongoc/mongoc-host-list.c \
	src/mongoc/mongoc-init.c \
	src/mongoc/mongoc-gridfs.c \
	src/mongoc/mongoc-gridfs-file.c \
	src/mongoc/mongoc-gridfs-file-page.c \
	src/mongoc/mongoc-gridfs-file-list.c \
	src/mongoc/mongoc-index.c \
	src/mongoc/mongoc-list.c \
	src/mongoc/mongoc-log.c \
	src/mongoc/mongoc-matcher-op.c \
	src/mongoc/mongoc-matcher.c \
	src/mongoc/mongoc-memcmp.c \
	src/mongoc/mongoc-opcode.c \
	src/mongoc/mongoc-queue.c \
	src/mongoc/mongoc-read-concern.c \
	src/mongoc/mongoc-read-prefs.c \
	src/mongoc/mongoc-rpc.c \
	src/mongoc/mongoc-server-description.c \
	src/mongoc/mongoc-server-stream.c \
	src/mongoc/mongoc-set.c \
	src/mongoc/mongoc-socket.c \
	src/mongoc/mongoc-stream.c \
	src/mongoc/mongoc-stream-buffered.c \
	src/mongoc/mongoc-stream-file.c \
	src/mongoc/mongoc-stream-gridfs.c \
	src/mongoc/mongoc-stream-socket.c \
	src/mongoc/mongoc-topology.c \
	src/mongoc/mongoc-topology-description.c \
	src/mongoc/mongoc-topology-scanner.c \
	src/mongoc/mongoc-uri.c \
	src/mongoc/mongoc-util.c \
	src/mongoc/mongoc-version-functions.c \
	src/mongoc/mongoc-write-command.c \
	src/mongoc/mongoc-write-concern.c

if ENABLE_SSL
MONGOC_SOURCES_SHARED += \
	src/mongoc/mongoc-rand.c \
	src/mongoc/mongoc-scram.c \
	src/mongoc/mongoc-stream-tls.c \
	src/mongoc/mongoc-ssl.c
endif

if ENABLE_SASL
MONGOC_SOURCES_SHARED += src/mongoc/mongoc-sasl.c
endif

EXTRA_DIST += $(MONGOC_DEF_FILES)

