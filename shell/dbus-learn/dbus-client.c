#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <dbus/dbus.h>

#define CLIENT_BUS_NAME	"com.roman.client"
#define SERVER_BUS_NAME	"com.roman.server"

#define OBJECT_NAME		"/com/roman/simple_obj"
#define INTERFACE_NAME	"com.roman.dbus"
#define METHOD_NAME		"GetMoreInfo"

static void end(const char* msg)
{
	puts(msg);
	exit(1);
}

int main()
{
	int ret;
	char* data;
	DBusConnection *conn;
	DBusError dbus_error;
	DBusMessage *msg;
	DBusMessageIter args;
	//
	DBusMessage *request;
	DBusPendingCall *pending;

	// инциализация переменной ошибки
	dbus_error_init(&dbus_error);

	// соединение с шиной
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);
	if (!conn)
		end("conn err\n");

	// закрепить имя за соединением
	ret = dbus_bus_request_name(conn, CLIENT_BUS_NAME, 0, &dbus_error);
	if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
		end("dbus_bus_request_name err\n");
	
	// установка правила для ожидаемого сигнала
	dbus_bus_add_match(conn, "type='signal',interface='" INTERFACE_NAME "'", &dbus_error);
	dbus_connection_flush(conn);
	if (dbus_error_is_set(&dbus_error))
		end("dbus_bus_add_match err\n");

signal_wait:	
	// ожидание сигнала
	for (;;) {
		// ожидание возможности читать (блокировка)
		dbus_connection_read_write_dispatch(conn, -1);
		// получение сообщения из очереди (удаляет из очереди)
		msg = dbus_connection_pop_message(conn);
		if (!msg)
			continue;

		// проверка имени сигнала
		if (dbus_message_is_signal(msg, INTERFACE_NAME, "Test")) {
			ret = dbus_message_iter_init(msg, &args);
			// проверка типа сигнала
			if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_STRING) {
				dbus_message_iter_get_basic(&args, &data);
				printf("Got Signal with value: %s\n", data);
				dbus_message_unref(msg);
				// на запрос метода
				goto method_request;
			}		     			
		}
		
		dbus_message_unref(msg);
	}


method_request:	
	request = dbus_message_new_method_call(SERVER_BUS_NAME, OBJECT_NAME, INTERFACE_NAME, METHOD_NAME);
	if (!request)
		end("dbus_message_new_method_call err\n");

	// инциализация iter struct
	dbus_message_iter_init_append(msg, &args);

	// добавить строку к сообщению
	data = "method call";
	ret = dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &data);
	if (!ret)
		end("dbus_message_iter_append_basic err\n");

	// добавить сообщение в очередь на отправку с handler'ом для получения ответа
	ret = dbus_connection_send_with_reply(conn, request, &pending, -1);
	if (!ret)
		end("dbus_connection_send_with_reply err\n");
	if (!pending)
		end("pending err\n");

	// блокировка работы до завершения отправки сигнала
	dbus_connection_flush(conn);

	dbus_message_unref(request);
	
	// ожидание ответа
	dbus_pending_call_block(pending);

	// получение ответа
	msg = dbus_pending_call_steal_reply(pending);
	if (!msg)
		end("dbus_pending_call_steal_reply err\n");

	// освобождение handler ответа
	dbus_pending_call_unref(pending);

	// чтение ответа
	ret = dbus_message_iter_init(msg, &args);
	if (ret) {
		// проверка типа
		if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_BOOLEAN) {
			int stat;
			dbus_message_iter_get_basic(&args, &stat);

			// чтение следующего поля
			ret = dbus_message_iter_next(&args);
			if (ret) {
				// проверка типа
				if (dbus_message_iter_get_arg_type(&args) == DBUS_TYPE_INT32) {
					int lvl;
					dbus_message_iter_get_basic(&args, &lvl);

					printf("Got Reply: %d, %d\n", stat, lvl);
				}				
			}
		}
	}

	dbus_message_unref(msg);
	goto signal_wait;

	// dbus_connection_close(conn);

	return 0;
}
