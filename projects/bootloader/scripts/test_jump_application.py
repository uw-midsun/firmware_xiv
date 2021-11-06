# """This Module Tests the functions in jump_application.py"""

# import unittest
# import can

# from can_datagram import Datagram
# from can_datagram import DatagramSender
# from can_datagram import DatagramListener

# TEST_CHANNEL = "vcan0"

# TEST_PROTOCOL_VERSION = 1
# TEST_DATAGRAM_TYPE_ID = 1
# TEST_NODES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
# TEST_DATA = []

# class TestJumpApplication(unittest.TestCase):
#   """Tests jump application functions"""

#     def test_jump_application(self):
#         """Test the node ids jump to application"""
#         self.callback_triggered = False
#         self.message = []

#         sender = DatagramSender(channel=TEST_CHANNEL, receive_own_messages=True)
#         listener = DatagramListener(self.triggerCallback)
#         notifier = can.Notifier(sender.bus, [listener])

#         message = Datagram(
#             datagram_type_id=TEST_DATAGRAM_TYPE_ID,
#             node_ids=TEST_NODES,
#             data=bytearray(TEST_DATA))
#         sender.send(message)

#         timeout = time.time() + 10
#         while not self.callback_triggered:
#             if time.time() > timeout:
#                 break

#         self.assertEqual(self.message.serialize(), message.serialize())
#         self.assertEqual(self.callback_triggered, True)

#     def triggerCallback(self, msg):
#         self.message = msg
#         self.callback_triggered = True


# if __name__ == "__main__":
#   unittest.main()
