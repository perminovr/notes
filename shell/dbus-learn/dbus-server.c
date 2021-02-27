#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <dbus/dbus.h>

/*
signals, methods, properties

scheme:
"com.roman   /    com.roman.server.Get"
 ^-------^   ^    ^--------------^ ^-^
  service  object    interface    method
*/

#define SERVER_BUS_NAME	"com.roman.server"

#define OBJECT_NAME 	"/com/roman/simple_obj"
#define INTERFACE_NAME 	"com.roman.dbus"
#define METHOD_NAME		"GetMoreInfo"

// номер сообщения d-bus
static dbus_uint32_t serial = 0;  

static void end(const char* msg)
{
	puts(msg);
	exit(1);
}

void send_signal(DBusConnection *conn, const char *data)
{
	int ret;
	DBusMessage *msg;
	DBusMessageIter args;

	// создать новый signal
	msg = dbus_message_new_signal(OBJECT_NAME, INTERFACE_NAME, "Test");
	if (!msg)
		end("dbus_message_new_signal err\n");

	// инциализация iter struct
	dbus_message_iter_init_append(msg, &args);

	// добавить строку к сообщению
	ret = dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &data);
	if (!ret)
		end("dbus_message_iter_append_basic err\n");

	// добавить сообщение в очередь на отправку
	ret = dbus_connection_send(conn, msg, &serial);
	if (!ret)
		end("dbus_connection_send err\n");

	// блокировка работы до завершения отправки сигнала
	dbus_connection_flush(conn);

	// удалить msg
	dbus_message_unref(msg);
}

#define METHOD_RET_TYPE int

void reply_to_method_call(DBusConnection *conn, DBusMessage *msg, void *res)
{
	int ret;
	int stat = 1;
	DBusMessage* reply;
	DBusMessageIter args;
	METHOD_RET_TYPE *mret = (METHOD_RET_TYPE*)res;
	
	// подготовка ответа на запрос метода
	reply = dbus_message_new_method_return(msg);
	if (!reply)
		end("dbus_message_new_method_return err\n");
	
	// инциализация iter struct
	dbus_message_iter_init_append(reply, &args);

	// добавить статус обработки (необязательно, было в примере)
	ret = dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &stat);
	if (!ret)
		end("dbus_message_iter_append_basic bool err\n");

	// добавить результат работы метода
	ret = dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, mret);
	if (!ret)
		end("dbus_message_iter_append_basic int err\n");

	// добавить сообщение в очередь на отправку
	ret = dbus_connection_send(conn, reply, &serial);
	if (!ret)
		end("dbus_connection_send err\n");

	// блокировка работы до завершения отправки сигнала
	dbus_connection_flush(conn);

	// удалить reply
	dbus_message_unref(reply);
}

METHOD_RET_TYPE GetMoreInfo(const char *data)
{
	return 321;
}

/*

https://dbus.freedesktop.org/doc/api/html/

1) client запущен
2) server запущен
	- отправка сигнала
	- ожидание приема запроса метода
	- выполнение метода
	- отправка результата
	- выход

*/
int main()
{
	int ret;
	DBusConnection *conn;
	DBusMessage *msg;
	DBusMessageIter args;
	DBusError dbus_error;

	// инциализация переменной ошибки
	dbus_error_init(&dbus_error);

	// соединение с шиной
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error); // DBUS_BUS_SESSION
	if (!conn)
		end("conn err\n");

	// закрепить имя за соединением
	ret = dbus_bus_request_name(conn, SERVER_BUS_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, &dbus_error);
	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) // должен быть единственным 'primary owner'
		end("dbus_bus_request_name err\n");

	// отправка сигнала
	send_signal(conn, "abc123");
	
	// ожидание запроса метода
	for (;;) {
		// ожидание возможности читать (блокировка)
		dbus_connection_read_write_dispatch(conn, -1);
		// получение сообщения из очереди (удаляет из очереди)
		msg = dbus_connection_pop_message(conn);
		if (!msg)
			continue;

		// проверка вызова метода
		ret = dbus_message_is_method_call(msg, INTERFACE_NAME, METHOD_NAME);
		if (ret) {
			// получение аргументов вызова
			ret = dbus_message_iter_init(msg, &args);
			if (ret) {
				char *data;
				dbus_message_iter_get_basic(&args, &data);
				METHOD_RET_TYPE mret = GetMoreInfo(data);
				reply_to_method_call(conn, msg, &mret);
				dbus_message_unref(msg);
				break;
			}
		}
		dbus_message_unref(msg);
	}

	//dbus_connection_close(conn);

	return 0;
}
