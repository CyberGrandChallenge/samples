#!/usr/bin/env python
#
# Copyright (C) 2014 Narf Industries <info@narfindustries.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

from generator.actions import Actions
from random import choice, randint
from os.path import dirname, abspath, join
import string
import sys


sys.path.append(join(dirname(dirname(dirname(abspath(__file__)))), "support"))
from multipasssupport import MultiPassSupport

def random_alpha(a, b):
    return ''.join(choice(string.letters) for _ in range(randint(a, b)))

def random_string(a, b):
    chars = string.letters + string.digits
    return ''.join(choice(chars) for _ in range(randint(a, b)))

def random_digits(a, b):
    chars = string.digits
    return ''.join(choice(chars) for _ in range(randint(a, b)))


class MultiPass(Actions):

	INITIAL_CARD_AMOUNT = 1000
	DEBUG = True
	DEBUG_ISSUE = DEBUG or False
	DEBUG_PURCHASE = DEBUG or False
	DEBUG_HISTORY = DEBUG or False
	DEBUG_REFUND = DEBUG or False

	def start(self):
		self.state['mp'] = MultiPassSupport()

	def issue(self):
		if self.DEBUG_ISSUE:
			print('\nIssue')

		## INIT ##
		if self.DEBUG_ISSUE:
			print('INIT')
		# send packet header:
		# set: pkt_type INIT, op_code ISSUE, amount INITAL_CARD_AMOUNT
		msg_head = self.state['mp'].make_packet_head(pkt_type = 'INIT', op_code = 'ISSUE')
		if self.DEBUG_ISSUE:
			print(' write: msg_head: {0}'.format(msg_head))
		msg_issue = self.state['mp'].make_packet_data_issue(amount = self.INITIAL_CARD_AMOUNT)
		if self.DEBUG_ISSUE:
			print(' write: msg_issue: {0}'.format(msg_issue))
		msg = self.state['mp'].pack_packet_head(msg_head) + self.state['mp'].pack_packet_data_issue(msg_issue)
		self.write(msg)

		# build expected reply header, same as inital packet, except
		#  transaction_id, card_id and auth_code will be newly assigned
		new_card_id = self.state['mp'].get_next_card_id()
		new_auth_code = self.state['mp'].get_next_auth_code()
		new_transaction_id = self.state['mp'].get_next_transaction_id()
		msg_head = self.state['mp'].make_packet_head(
					card_id = new_card_id,
					auth_code = new_auth_code,
					pkt_type = 'INIT', 
					op_code = 'ISSUE',
					transaction_id = new_transaction_id)
		if self.DEBUG_ISSUE:
			print(' read: msg_head: {0}'.format(msg_head))
		msg = self.state['mp'].pack_packet_head(msg_head)
		# read reply packet header
		self.read(length=len(msg), expect=msg)

		# add card to C_LIST
		new_card = {'card_id': new_card_id,
					'auth_code': new_auth_code,
					'balance': self.INITIAL_CARD_AMOUNT}
		self.state['mp'].add_card(new_card)

		# add transaction to T_LIST
		new_issue = {'amount': self.INITIAL_CARD_AMOUNT}
		new_transaction = self.state['mp'].make_transaction(
											card_id = new_card_id, 
											state = 'INIT', 
											op_code = 'ISSUE', 
											transaction_id = new_transaction_id,
											details = new_issue)
		self.state['mp'].add_transaction(new_transaction)

		## FIN ##
		if self.DEBUG_ISSUE:
			print('\nFIN')
		msg_head = self.state['mp'].make_packet_head(
					card_id = new_card_id,
					auth_code = new_auth_code,
					pkt_type = 'FIN', 
					op_code = 'ISSUE',
					transaction_id = new_transaction_id)
		if self.DEBUG_ISSUE:
			print(' write then read: msg_head: {0}'.format(msg_head))
		msg = self.state['mp'].pack_packet_head(msg_head)
		self.write(msg)
		self.read(length=len(msg), expect=msg)

		# update transaction to FIN
		self.state['mp'].update_transaction_state(new_transaction_id, 'FIN')

	def purchase(self):
		if self.DEBUG_PURCHASE:
			print('\nPurchase')

		## AUTH ##
		# select a valid card
		if self.DEBUG_PURCHASE:
			print('AUTH')
		card = self.state['mp'].get_random_card()
		if card == -1:
			return

		msg_head = self.state['mp'].make_packet_head(
					card_id = card['card_id'],
					auth_code = card['auth_code'],
					pkt_type = 'AUTH', 
					op_code = 'PURCHASE')
		if self.DEBUG_PURCHASE:
			print(' write: msg_head: {0}'.format(msg_head))
		msg = self.state['mp'].pack_packet_head(msg_head)
		self.write(msg)

		# build expected reply header, same as inital packet, except
		#  transaction_id will be newly assigned
		new_transaction_id = self.state['mp'].get_next_transaction_id()
		msg_head = self.state['mp'].make_packet_head(
					card_id = card['card_id'],
					auth_code = card['auth_code'],
					pkt_type = 'AUTH', 
					op_code = 'PURCHASE',
					transaction_id = new_transaction_id)
		if self.DEBUG_PURCHASE:
			print(' read: msg_head: {0}'.format(msg_head))
		msg = self.state['mp'].pack_packet_head(msg_head)
		# read reply packet header
		self.read(length=len(msg), expect=msg)

		# add transaction to T_LIST
		new_transaction = self.state['mp'].make_transaction(
											card_id = card['card_id'], 
											state = 'AUTH', 
											op_code = 'PURCHASE', 
											transaction_id = new_transaction_id)
		self.state['mp'].add_transaction(new_transaction)

		## OPS ##

		if self.DEBUG_PURCHASE:
			print('\nOPS')
		msg_head = self.state['mp'].make_packet_head(
					card_id = card['card_id'],
					auth_code = card['auth_code'],
					pkt_type = 'OPS', 
					op_code = 'PURCHASE',
					transaction_id = new_transaction_id)

		if self.DEBUG_PURCHASE:
			print(' write: msg_head: {0}'.format(msg_head))

		new_purchase_amt = randint(1, 999)
		msg_purchase = self.state['mp'].make_packet_data_purchase(new_purchase_amt)
		if self.DEBUG_PURCHASE:
			print(' write: msg_purchase: {0}'.format(msg_purchase))
		msg = self.state['mp'].pack_packet_head(msg_head) + self.state['mp'].pack_packet_data_purchase(msg_purchase)
		self.write(msg)

		# expected reply header, same as inital packet head, unless purchase causes ISF
		# When card balance < new_purchase_amt, the service will return an ISF error and exit.

		ISF = False
		if new_purchase_amt > card['balance']:
			ISF = True
			msg_head = self.state['mp'].make_packet_head(
						card_id = card['card_id'],
						auth_code = card['auth_code'],
						pkt_type = 'OPS', 
						op_code = 'PURCHASE',
						status = 'ERRNO_MP_PURCHASE_ISF',
						transaction_id = new_transaction_id)

		if self.DEBUG_PURCHASE:
			print(' read: msg_head: {0}'.format(msg_head))
		msg = self.state['mp'].pack_packet_head(msg_head)
		# read reply packet header
		self.read(length=len(msg), expect=msg)

		if ISF == True:
			return -1

		# update balance of card
		self.state['mp'].card_purchase(card['card_id'], new_purchase_amt)

		# set recharge details
		self.state['mp'].update_transaction_details(new_transaction_id, msg_purchase)

		# update transaction to OPS
		self.state['mp'].update_transaction_state(new_transaction_id, 'OPS')

		## FIN ##
		if self.DEBUG_PURCHASE:
			print('\nFIN')
		msg_head = self.state['mp'].make_packet_head(
					card_id = card['card_id'],
					auth_code = card['auth_code'],
					pkt_type = 'FIN', 
					op_code = 'PURCHASE',
					transaction_id = new_transaction_id)
		if self.DEBUG_PURCHASE:
			print(' write then read: msg_head: {0}'.format(msg_head))
		msg = self.state['mp'].pack_packet_head(msg_head)
		self.write(msg)
		self.read(length=len(msg), expect=msg)

		# update transaction to FIN
		self.state['mp'].update_transaction_state(new_transaction_id, 'FIN')

	def history(self):
		if self.DEBUG_HISTORY:
			print('\nHistory')

		## AUTH ##
		# select a valid card
		if self.DEBUG_HISTORY:
			print('AUTH')
		card = self.state['mp'].get_random_card()
		if card == -1:
			return

		msg_head = self.state['mp'].make_packet_head(
					card_id = card['card_id'],
					auth_code = card['auth_code'],
					pkt_type = 'AUTH', 
					op_code = 'HISTORY')
		if self.DEBUG_HISTORY:
			print(' write: msg_head: {0}'.format(msg_head))
		msg = self.state['mp'].pack_packet_head(msg_head)
		self.write(msg)

		# build expected reply header, same as inital packet, except
		#  transaction_id will be newly assigned
		new_transaction_id = self.state['mp'].get_next_transaction_id()
		msg_head = self.state['mp'].make_packet_head(
					card_id = card['card_id'],
					auth_code = card['auth_code'],
					pkt_type = 'AUTH', 
					op_code = 'HISTORY',
					transaction_id = new_transaction_id)
		if self.DEBUG_HISTORY:
			print(' read: msg_head: {0}'.format(msg_head))
		msg = self.state['mp'].pack_packet_head(msg_head)
		# read reply packet header
		self.read(length=len(msg), expect=msg)

		# add transaction to T_LIST
		new_transaction = self.state['mp'].make_transaction(
											card_id = card['card_id'], 
											state = 'AUTH', 
											op_code = 'HISTORY', 
											transaction_id = new_transaction_id)
		self.state['mp'].add_transaction(new_transaction)

		## OPS ##
		if self.DEBUG_HISTORY:
			print('\nOPS')
		msg_head = self.state['mp'].make_packet_head(
					card_id = card['card_id'],
					auth_code = card['auth_code'],
					pkt_type = 'OPS', 
					op_code = 'HISTORY',
					transaction_id = new_transaction_id)

		if self.DEBUG_HISTORY:
			print(' write: msg_head: {0}'.format(msg_head))
		history_cnt = randint(1, 20)
		msg_history = self.state['mp'].make_packet_data_history(history_cnt)
		if self.DEBUG_HISTORY:
			print(' write: msg_history: {0}'.format(msg_history))

		msg = self.state['mp'].pack_packet_head(msg_head) + self.state['mp'].pack_packet_data_history(msg_history)	
		self.write(msg)

		# set history details
		self.state['mp'].update_transaction_details(new_transaction_id, msg_history)

		# update transaction to OPS
		self.state['mp'].update_transaction_state(new_transaction_id, 'OPS')

		# expected reply header, same as sent packet msg_head
		if self.DEBUG_HISTORY:
			print(' read: msg_head: {0}'.format(msg_head))
		# read reply packet header and reply history count (the number of historical records that will be sent)
		history_cnt_avail = self.state['mp'].get_history_count_avail(card['card_id'], history_cnt)
		msg_history = self.state['mp'].make_packet_data_history(history_cnt_avail)

		if self.DEBUG_HISTORY:
			print(' read: msg_history: {0}'.format(msg_history))

		msg = self.state['mp'].pack_packet_head(msg_head) + self.state['mp'].pack_packet_data_history(msg_history)
		self.read(length=len(msg), expect=msg)


		# read history_cnt_avail historical transactions and details
		# TODO: if history_cnt_avail < 0, server will return ERRNO_MP_NO_HISTORY and exit.
		for tr in self.state['mp'].get_historical_transactions_iter(card['card_id'], history_cnt_avail):
			# read transaction header (minus details)
			if self.DEBUG_HISTORY:
				print(' read: tr and details: {0}'.format(tr))

			msg = self.state['mp'].pack_transaction_head(tr)
			self.read(length=len(msg), expect=msg)

			# read transaction details
			if tr['details'] == {}:
				return -1
			msg = self.state['mp'].pack_transaction_details(tr)
			if msg == -1:
				if self.DEBUG_HISTORY:
					print(' error: empty transaction details for tr: {0}'.format(tr))
				# return msg
			else:
				self.read(length=len(msg), expect=msg)

		## FIN ##
		# if self.DEBUG_HISTORY:
		# 	print('\nFIN')
		# msg_head = self.state['mp'].make_packet_head(
		# 			card_id = card['card_id'],
		# 			auth_code = card['auth_code'],
		# 			pkt_type = 'FIN', 
		# 			op_code = 'HISTORY',
		# 			transaction_id = new_transaction_id)
		# if self.DEBUG_HISTORY:
		# 	print(' write then read: msg_head: {0}'.format(msg_head))
		# msg = self.state['mp'].pack_packet_head(msg_head)
		# self.write(msg)
		# self.read(length=len(msg), expect=msg)

		# # update transaction to FIN
		# self.state['mp'].update_transaction_state(new_transaction_id, 'FIN')

	def refund_auth_ops(self):
		if self.DEBUG_REFUND:
			print('\nRefund Auth Ops')

		## AUTH ##
		# select a valid card that was used to make a purchase
		if self.DEBUG_REFUND:
			print('AUTH')
		card = self.state['mp'].get_random_card_with_purchase()
		# if no cards made a purchase, exit
		if card == {}:
			if self.DEBUG_REFUND:
				print(' no cards made a purchase.')
			return

		msg_head = self.state['mp'].make_packet_head(
					card_id = card['card_id'],
					auth_code = card['auth_code'],
					pkt_type = 'AUTH', 
					op_code = 'REFUND')
		if self.DEBUG_REFUND:
			print(' write: msg_head: {0}'.format(msg_head))
		msg = self.state['mp'].pack_packet_head(msg_head)
		self.write(msg)

		# build expected reply header, same as inital packet, except
		#  transaction_id will be newly assigned
		new_transaction_id = self.state['mp'].get_next_transaction_id()
		msg_head = self.state['mp'].make_packet_head(
					card_id = card['card_id'],
					auth_code = card['auth_code'],
					pkt_type = 'AUTH', 
					op_code = 'REFUND',
					transaction_id = new_transaction_id)
		if self.DEBUG_REFUND:
			print(' read: msg_head: {0}'.format(msg_head))
		msg = self.state['mp'].pack_packet_head(msg_head)
		# read reply packet header
		self.read(length=len(msg), expect=msg)

		# add transaction to T_LIST
		new_transaction = self.state['mp'].make_transaction(
											card_id = card['card_id'], 
											state = 'AUTH', 
											op_code = 'REFUND', 
											transaction_id = new_transaction_id)
		self.state['mp'].add_transaction(new_transaction)

		## OPS ##
		if self.DEBUG_REFUND:
			print('\nOPS')
		msg_head = self.state['mp'].make_packet_head(
					card_id = card['card_id'],
					auth_code = card['auth_code'],
					pkt_type = 'OPS', 
					op_code = 'REFUND',
					transaction_id = new_transaction_id)

		if self.DEBUG_REFUND:
			print(' write: msg_head: {0}'.format(msg_head))
		msg_refund = self.state['mp'].get_random_refund_data_by_card(card)
		if msg_refund == {}:
			return -1

		if self.DEBUG_REFUND:
			print(' write: msg_refund: {0}'.format(msg_refund))

		msg = self.state['mp'].pack_packet_head(msg_head) + self.state['mp'].pack_packet_data_refund(msg_refund)	
		self.write(msg)

		# set history details
		self.state['mp'].update_transaction_details(new_transaction_id, msg_refund)

		# update transaction to OPS
		self.state['mp'].update_transaction_state(new_transaction_id, 'OPS')

		# expected reply header, same as sent packet msg_head
		if self.DEBUG_REFUND:
			print(' read: msg_head: {0}'.format(msg_head))

		msg = self.state['mp'].pack_packet_head(msg_head)
		self.read(length=len(msg), expect=msg)

		# refund cost to card
		self.state['mp'].refund_purchase_to_card(msg_refund)

		# delete purchase details from refund transaction (del transaction in refund_fin)
		self.state['mp'].delete_refunded_purchase_details(msg_refund)

		self.state['refund_card'] = card
		self.state['refund_tid'] = new_transaction_id
		self.state['refund_data'] = msg_refund

	# def refund_fin(self):
	# 	if self.DEBUG_REFUND:
	# 		print('\nRefund Fin')

	# 	card = self.state['refund_card']
	# 	new_transaction_id = self.state['refund_tid']
	# 	msg_refund = self.state['refund_data']

	# 	## FIN ##
	# 	if self.DEBUG_REFUND:
	# 		print('\nFIN')
	# 	msg_head = self.state['mp'].make_packet_head(
	# 				card_id = card['card_id'],
	# 				auth_code = card['auth_code'],
	# 				pkt_type = 'FIN', 
	# 				op_code = 'REFUND',
	# 				transaction_id = new_transaction_id)
	# 	if self.DEBUG_REFUND:
	# 		print(' write then read: msg_head: {0}'.format(msg_head))
	# 	msg = self.state['mp'].pack_packet_head(msg_head)
	# 	self.write(msg)
	# 	self.read(length=len(msg), expect=msg)

	# 	# destroy refunded transaction
	# 	self.state['mp'].delete_refunded_purchase_transaction(msg_refund)

	# 	# update transaction to FIN
	# 	self.state['mp'].update_transaction_state(new_transaction_id, 'FIN')

