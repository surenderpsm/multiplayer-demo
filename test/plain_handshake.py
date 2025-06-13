import socket

SERVER_IP = "127.0.0.1"
SERVER_PORT = 9000

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(2)  # 2s timeout

    try:
        # Send handshake
        message = "HELLO".encode()
        sock.sendto(message, (SERVER_IP, SERVER_PORT))
        print(f"Sent: {message.decode()}")

        # Receive welcome
        data, _ = sock.recvfrom(1024)
        print(f"Received: {data.decode()}")
    
    except socket.timeout:
        print("No response from server.")
    
    finally:
        sock.close()

if __name__ == "__main__":
    main()
# This script sends a handshake message to a server and waits for a response.
# It handles the case where the server does not respond within a timeout period.