#!/usr/bin/env python
import tornado.httpserver
import tornado.websocket
import tornado.ioloop
import tornado.web
import os
import sqlite3
from datetime import datetime

wss = []

class WSHandler(tornado.websocket.WebSocketHandler):
    def check_origin(self, origin):
        return True

    def open(self):
        print('New user is connected.\n')
        if self not in wss:
            wss.append(self)

    def on_close(self):
        print('Connection closed\n')
        if self in wss:
            wss.remove(self)

def init_db():
    conn = sqlite3.connect('temperature_data.db')
    c = conn.cursor()
    c.execute('''
        CREATE TABLE IF NOT EXISTS temperature (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT,
            temperature REAL
        )
    ''')
    conn.commit()
    conn.close()

def insert_temperature(timestamp, temperature):
    conn = sqlite3.connect('temperature_data.db')
    c = conn.cursor()
    c.execute('INSERT INTO temperature (timestamp, temperature) VALUES (?, ?)', (timestamp, temperature))
    conn.commit()
    conn.close()

def ws_send(message):
    for ws in wss:
        if not ws.ws_connection.stream.socket:
            print("Web socket does not exist anymore!!!")
            wss.remove(ws)
        else:
            ws.write_message(message)

def read_temp_and_send_ws():
    try:
        conn = sqlite3.connect('temperature_data.db')
        c = conn.cursor()
        c.execute('SELECT * FROM temperature ORDER BY timestamp DESC LIMIT 1')
        result = c.fetchone()
        conn.close()
        if result:
            timestamp, temperature = result[1], result[2]
            ws_send(f"Temperature: {temperature}Â°C at {timestamp}")
    except Exception as e:
        print(f"Error reading temperature from database: {e}")

application = tornado.web.Application([
    (r'/ws', WSHandler),
])

if __name__ == "__main__":
    init_db()  
    interval_msec = 2000
    http_server = tornado.httpserver.HTTPServer(application)
    http_server.listen(8888)
    main_loop = tornado.ioloop.IOLoop.instance()
    sched_temp = tornado.ioloop.PeriodicCallback(read_temp_and_send_ws, interval_msec, io_loop=main_loop)
    sched_temp.start()
    main_loop.start()
