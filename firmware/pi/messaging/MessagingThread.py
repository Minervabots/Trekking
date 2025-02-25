from queue import Queue
from serial import Serial, SerialException
from threading import Thread
from typing import List, Dict, Any
from .MessageHandler import MessageHandler
from .MessageCodes import MessageCodes
import io

class MessagingThread(Thread):
    HandlerCollection = Dict[int, MessageHandler]
    
    def __init__(self, handlers : MessageHandler):
        super(MessagingThread, self).__init__()
        self._sendQueue = Queue()

        self._handlers = {}
        for handler in handlers:
            opCode = int(handler.opCode)
            if(opCode not in self._handlers):
                self._handlers[opCode] = handler
      
        self.setPort("")
        self.setOptions(9600, 0.1, 1000);
        self.setSeparators(",", ";")
        self.isRunning = False
        
    def setPort(self, port):
        self.port = port
        
    def setOptions(self, baudRate : int = 9600, timeout : float = 0.1, bufferQueueSize : int = 1000):
        self.baudRate = baudRate
        self.timeout = timeout
        self.bufferQueueSize = bufferQueueSize
        
    def setSeparators(self, fieldSeparator : str, messageSeparator : str):
        self.fieldSeparator = fieldSeparator
        self.messageSeparator = messageSeparator
        
    def start(self):
        if self.port == "":
            raise ValueError("A porta deve ser definida antes de iniciar a thread")
        
        
        try:
            self.stream = Serial(self.port, baudrate=self.baudRate, timeout=self.timeout)
        except SerialException:
            self.failed = True
            print("Não foi possivel abrir a porta. Nenhuma funcionalidade dessa classe será executada")
            return
            
        self.isRunning = True
        super(MessagingThread, self).start()
        
    def close(self):
        self.isRunning = False
    
    def clearSendQueue(self):
        self._sendQueue.queue.clear()
      
    def send(self, opCode, *args):
        if (self.failed):
            return
            
        message = str(int(opCode)) + self.fieldSeparator + self.fieldSeparator.join(str(field) for field in args) + self.messageSeparator
        self._sendQueue.put(message.encode())

    def run(self):
        receivedFirstFullMessage = False
        while self.isRunning and self.stream.isOpen():
            # Primeiro envia as mensagens
            if not self._sendQueue.empty():
                message = self._sendQueue.get()
                #print(message)
                self.stream.write(message)
        
            if(self.stream.inWaiting() == 0):
                continue

            # Ignora a primeira mesagem pois muito provavelmente não vamos receber ela completa
            while not(receivedFirstFullMessage):
                for c in self.stream.read().decode():
                    if c == self.messageSeparator:
                        receivedFirstFullMessage = True
                        break;

            hasFullMessage = False
            decodedBuffer = ""
        
            # Espera receber o byte indicador de fim de mensagem para processar
            # a mensagem completa
            while not hasFullMessage:
                for c in self.stream.read().decode():
                    if c == self.messageSeparator:
                        hasFullMessage = True
                        break
                    decodedBuffer += c
        
            # Remove bytes inválidos da mensagem e separa cada campo em uma lista
            message = decodedBuffer.strip('').strip('\n\r').split(self.fieldSeparator)
            if(len(message) == 0 or not message[0].isdigit()):
                #print(message)
                continue
        
            opCode = int((message[0]))
            if(opCode in self._handlers):
                self._handlers[opCode].handle(self, message)
            else:
                self.send(CommonMessageCodes.ERROR, "Handler invalido")
      
        # Só fecha a stream quando sair do loop
        # Evita erros devido a thread não ser sincronizada
        self.stream.close()