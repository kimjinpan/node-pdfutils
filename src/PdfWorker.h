/*
 * PdfWorker.h
 * Copyright (C) 2014 tox <tox@rootkit>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PDFWORKER_H
#define PDFWORKER_H

#include <nan.h>
#include <uv.h>
#include <list>
#include "PdfController.h"
#if (NODE_MODULE_VERSION > 0x000B)
#	define UV_ASYNC_STATUS
#else
#	define UV_ASYNC_STATUS , int status
#endif


template <class T>
class PdfWorker : public NanAsyncWorker {
private:
	T *_controller;
	uv_async_t async;
	uv_mutex_t mutex;
	std::list<void *> intermediate;


	static void handleIntermediate(uv_async_t *handle UV_ASYNC_STATUS) {
		PdfWorker *self = (PdfWorker *)handle->data;
		void *data;

		while(true) {
			uv_mutex_lock(&self->mutex);
			if(self->intermediate.empty()) {
				uv_mutex_unlock(&self->mutex);
				break;
			} else {
				data = self->intermediate.front();
				self->intermediate.pop_front();
				uv_mutex_unlock(&self->mutex);
			}
			self->HandleIntermediate(data);
		}
	}

public:
	PdfWorker(T *controller, NanCallback *callback) : NanAsyncWorker(callback) {
		_controller = controller;

		uv_loop_t *loop = uv_default_loop();
		uv_async_init(loop, &async, handleIntermediate);
		async.data = this;
		uv_mutex_init(&mutex);
	}

	virtual ~PdfWorker() {
		uv_unref((uv_handle_t*)&this->async);
		uv_mutex_destroy(&this->mutex);
	}

	T *controller() {
		return _controller;
	}

	void CallIntermediate(void *data) {
		uv_mutex_lock(&this->mutex);
		intermediate.push_back(data);
		uv_mutex_unlock(&this->mutex);
	}

	virtual void HandleIntermediate(void *data) {}
};

#endif /* !PDFWORKER_H */
