import socket

class GameClient:
    def __init__(self, host='127.0.0.1', port=8080):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((host, port))

    def send_action(self, action):
        self.sock.sendall((action + "\n").encode())

    def receive(self):
        return self.sock.recv(1024).decode()

    def close(self):
        self.sock.close()
