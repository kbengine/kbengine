import logging
import socket


logger = logging.getLogger(__name__)


class WebSocketFactory(object): 
    def __init__(self, request):
        self.request = request

    def is_websocket(self):
        """check the websocket"""
        if self.request.META.get(
            'HTTP_UPGRADE', ""
        ).lower() == 'websocket':
            return True
        else:
            return False

    def get_websocket_version(self):
        if 'HTTP_SEC_WEBSOCKET_KEY1' in self.request.META:
            protocol_version = '76'
            if 'HTTP_SEC_WEBSOCKET_KEY2' not in self.request.META:
                raise ValueError('HTTP_SEC_WEBSOCKET_KEY2 NOT FOUND')
        elif 'HTTP_SEC_WEBSOCKET_KEY' in self.request.META:
            protocol_version = '13'
        else:
            protocol_version = '75'
        return protocol_version

    def create_websocket(self):
        raise NotImplementedError