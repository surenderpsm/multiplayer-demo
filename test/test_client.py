import socket

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(("127.0.0.1", 0))  # OS assigns port

s.sendto(b"HELLO", ("127.0.0.1", 9000))
print("Sent HELLO")

data, _ = s.recvfrom(1024)
print("Received:", data.decode())

s.sendto(b"UPDATE:1:100:100", ("127.0.0.1", 9000))
print("Sent update")
