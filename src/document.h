#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <vector>
#include <queue>
#include <node.h>
#include <poppler.h>
#include "page.h"
#include "page_job.h"

struct Chunk {
	PageJob *pj;
	char *value;
	int length;
};

class Document : public node::ObjectWrap {
	public:
		static void Init(v8::Handle<v8::Object> target);
		v8::Persistent<v8::Function> loadCb;
		PopplerDocument *doc;
		std::vector<Page*> *pages;
		std::queue<PageJob*> jobs;
		uv_mutex_t jobMutex;
		bool needMessage;
		uv_sem_t messageSem;
		std::queue<Chunk*> chunks;
		uv_mutex_t chunkMutex;
		uv_work_t worker;
		uv_async_t message_finished;
		uv_async_t message_data;
		
		void addJob(PageJob *job);
		void addChunk(PageJob *job, const unsigned char* data, unsigned int length);

	private:
		v8::Persistent<v8::Object> jsbuffer;
		char *buffer;
		int buflen;

		std::queue<PageJob*> finishedJobs;
		Document(v8::Persistent<v8::Object> &buffer, v8::Persistent<v8::Function> &loadCb);
		~Document();

		void loaded();
		static v8::Handle<v8::Value> New(const v8::Arguments& args);
		v8::Handle<v8::Value> getProperty(const char *key);
		
		static void BackgroundLoad(uv_work_t* req);
		static void BackgroundLoaded(uv_work_t* req);

		static void Worker(uv_work_t *handle);
		static void WorkerFinished(uv_async_t *handle, int status /*UNUSED*/);
		static void WorkerChunk(uv_async_t *handle, int status /*UNUSED*/);
		static void WorkerClean(uv_work_t *handle);
};

#endif
