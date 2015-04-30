#!/usr/bin/env python

from generator.actions import Actions
import random


class ProposalGenerator(Actions):


    def start(self):

        self.prop_name = ''
        self.profit = 0
        self.profit_str = '0.00'
        self.prop_setup=False
        self.pop_start = Dates()
        self.pop_end = Dates()
        self.resource_entry_count=0
        self.resource_entries=[]
        self.laborgrade=[6550, 10010, 10522, 11069, 13599]
        self.fill_performed=False
        self.limits_reached=False

        self.fringe=3125;
        self.oh=6512;
        self.gna=811;

        self.read_main_menu_text()

    def read_main_menu_text(self):

        self.read(delim='\n')
        self.read(delim='\n', expect='Proposal Pricing System\n')
        self.read(delim='\n')
        self.read(delim='\n', expect='A - Setup Proposal')
        self.read(delim='\n', expect='B - Display Indirect Rates')
        self.read(delim='\n', expect='C - Display Labor Rates')
        self.read(delim='\n', expect='D - Enter Labor Estimates')
        self.read(delim='\n', expect='E - Display Labor Estimates')
        self.read(delim='\n', expect='F - Price Proposal')
        self.read(delim='\n', expect='G - Exit System')

    def menu(self):

        pass
        

    def display_indirect(self):

        self.state_node='display_indirect'

        self.read(delim=':', expect=':')
        self.write(random.choice('bB') + '\n')
        self.read(delim='\n')
        self.read(delim='\n')

        self.read(delim='\n',expect='                   Indirect Rates\n')
        self.read(delim='\n',expect='**********************************************************\n')
        self.read(delim='\n',expect='Fiscal Year\tFringe\t\tOH\t\tG&A\n')
        self.read(delim='\n',expect='2015\t\t31.25%\t\t65.12%\t\t8.11%\t\t\n')
        self.read(delim='\n',expect='2016\t\t31.87%\t\t66.42%\t\t8.27%\t\t\n')
        self.read(delim='\n',expect='2017\t\t32.50%\t\t67.74%\t\t8.43%\t\t\n')
        self.read(delim='\n',expect='2018\t\t33.15%\t\t69.09%\t\t8.59%\t\t\n')
        self.read(delim='\n',expect='2019\t\t33.81%\t\t70.47%\t\t8.76%\t\t\n')
        self.read(delim='\n',expect='**********************************************************\n')

        self.read_main_menu_text()

    def display_laborrates(self):

        self.state_node='display_laborrates'

        self.read(delim=':', expect=':')
        self.write(random.choice('cC') + '\n')
        self.read(delim='\n')
        self.read(delim='\n')
        self.read(delim='\n',expect='            Direct Engineering Labor Rates\n')
        self.read(delim='\n',expect='**********************************************************\n')
        self.read(delim='\n',expect='Fiscal Year\tE01\tE02\tE03\tE04\tE05\n')
        self.read(delim='\n',expect='2015\t\t$65.50\t$100.10\t$105.22\t$110.69\t$135.99\t\n')
        self.read(delim='\n',expect='2016\t\t$66.81\t$102.10\t$107.32\t$112.90\t$138.70\t\n')
        self.read(delim='\n',expect='2017\t\t$68.14\t$104.14\t$109.46\t$115.15\t$141.47\t\n')
        self.read(delim='\n',expect='2018\t\t$69.50\t$106.22\t$111.64\t$117.45\t$144.29\t\n')
        self.read(delim='\n',expect='2019\t\t$70.89\t$108.34\t$113.87\t$119.79\t$147.17\t\n')
        self.read(delim='\n',expect='**********************************************************\n')

        self.read_main_menu_text()

    def read_prop_setup_menu(self):

        self.read(delim='\n')
        self.read(delim='\n', expect='Proposal Setup\n')
        self.read(delim='\n')
        self.read(delim='\n', expect='A - Display Proposal Info')
        self.read(delim='\n', expect='B - Enter Proposal Name')
        self.read(delim='\n', expect='C - Enter Proposal Fee/Profit')
        self.read(delim='\n', expect='D - Enter Proposal POP')
        self.read(delim='\n', expect='E - Exit Proposal Setup')

    def prop_setup(self):

        self.state_node='prop_setup'

        self.read(delim=':', expect=':')

        self.write(random.choice('aA') + '\n')
        self.read_prop_setup_menu()

    def read_prop_info(self):

        self.read(delim='\n', expect='\n')
        self.read(delim='\n', expect='\n')
        self.read(delim='\n',expect='*************************************************\n')
        self.read(delim='\n',expect='Title: ' + self.prop_name + '\n')
        self.read(delim='\n',expect='Start Date: ' + self.pop_start.print_date())
        self.read(delim='\n',expect='End Date: ' + self.pop_end.print_date())
        self.read(delim='\n',expect='Fee/Profit: ' + self.profit_str + '\n')
        self.read(delim='\n',expect='*************************************************\n')

    def display_prop_info(self):


        self.read(delim=':', expect=':')
        self.write(random.choice('aA') + '\n')

        self.read_prop_info()
        self.read_prop_setup_menu()

    def return_to_main_menu(self):

        self.read(delim=':', expect=':')
        self.write(random.choice('eE') + '\n')

        if self.prop_setup==False:
            self.read_prop_info()
            self.read(length=72, expect='Proposal is not completely setup.  Are you sure you wish to exit setup? ')
            self.write('y\n')

        self.read_main_menu_text()


    def exit(self):

        self.read(delim=':', expect=':')
        self.write(random.choice('gG') + '\n')


    def enter_title(self):

        self.read(delim=':', expect=':')
        self.write(random.choice('bB') + '\n')

        # actual input expected to be 100 bytes or less, do 105 to test bounds checking
        temp_str=''.join(random.choice('abcdefghijklmnop') for _ in range(65))

        if len(temp_str) > 100:
            self.prop_name=temp_str[:100]
        else:
            self.prop_name=temp_str

        self.read(delim=':', expect='Enter the proposal name (max 100 chars):')
        self.write(temp_str + '\n')
        self.read_prop_setup_menu()

    def enter_profit(self):


        self.read(delim=':', expect=':')
        self.write(random.choice('cC') + '\n')

        while (1):

            self.read(length=24, expect='Enter the % fee/profit: ')

            # allowable profit is 0-15.0%  do bigger to check bounds
            self.profit = random.randint(0,1200)

            self.profit_str = '{:.2f}'.format(self.profit/100.0)

            self.write(self.profit_str+'\n')

            if self.profit > 1500:
                self.read(delim='\n', expect='\n')
                self.read(delim='\n', expect='Invalid fee entered.  Valid range is 0-15\n')
            else:
                break

        self.read_prop_setup_menu()


    def enter_pop(self):

        self.pop_start=Dates().random_date_from_range(2015, 2016)

        self.read(delim=':', expect=':')
        self.write(random.choice('dD') + '\n')

        self.read(length=43, expect='Enter the program start date (MM/DD/YYYY): ')
        self.write(self.pop_start.print_date()+'\n')

        while (1):
            self.pop_end=Dates().random_date()
            self.read(length=41, expect='Enter the program end date (MM/DD/YYYY): ')
            self.write(self.pop_end.print_date()+'\n')

            if self.pop_end.compare_dates(self.pop_start) == 1:
                self.read_prop_setup_menu()
                break
            else:
                self.read(delim='\n', expect='End date must be after start date\n')
                self.read(delim='\n', expect='\n')


    def Setup_Path1(self):

        if self.prop_setup==True:
            return

        self.enter_profit()

        self.enter_title()

        self.enter_pop()

        self.prop_setup=True

    def Setup_Path2(self):

        if self.prop_setup==True:
            return

        self.enter_profit()

        self.enter_pop()

        self.enter_title()

        self.prop_setup=True

    def Setup_Path3(self):

        if self.prop_setup==True:
            return

        self.enter_pop()

        self.enter_profit()

        self.enter_title()
        self.prop_setup=True

    def Setup_Path4(self):

        if self.prop_setup==True:
            return

        self.enter_pop()

        self.enter_title()

        self.enter_profit()

        self.prop_setup=True

    def Setup_Path5(self):

        if self.prop_setup==True:
            return

        self.enter_title()

        self.enter_pop()

        self.enter_profit()

        self.prop_setup=True

    def Setup_Path6(self):

        if self.prop_setup==True:
            return

        self.enter_title()

        self.enter_profit()

        self.enter_pop()

        self.prop_setup=True

    def fill_resources(self):


        if self.prop_setup==False:
            self.enter_data(1)
            self.read_main_menu_text()
            return

        if self.fill_performed==False:
        
            self.fill_performed=True

            num_resources = random.randint(4, 50)

            self.enter_data(num_resources)
        
            self.read_main_menu_text()        

    def enter_one_resource(self):

        self.enter_data(1)
        self.read_main_menu_text()

    def enter_data(self, count):

        self.read(delim=':', expect=':')
        self.write(random.choice('dD') + '\n')
  
        if self.prop_setup==False:
            self.read(delim='\n', expect='\n')
            self.read(delim='\n', expect='Proposal must be setup before resource estimates can be entered\n')
            self.read(delim='\n', expect='\n')
            return

        for x in range(0, count):

            if self.resource_entry_count >= 600:
                self.read(length=35, expect='\nProposal resource limits reached\n\n')
                self.limits_reached=True
                self.fill_performed=True
                return

            eng_grade = random.randint(1, 5)
            eng_hours = random.randint(1, 5000)

            self.read(delim='\n', expect='\n')
            self.read(length=70, expect='Enter the estimate\'s start date (MM/DD/YYYY) or a blank line to exit: ')
            self.write(self.pop_start.print_date()+'\n')
            self.read(delim='\n', expect='\n')

            self.read(length=44, expect='Enter the estimate\'s end date (MM/DD/YYYY): ')
            self.write(self.pop_end.print_date()+'\n')
            self.read(delim='\n', expect='\n')

            self.read(length=35, expect='Enter the engineering grade (1-5): ')
            self.write('{}'.format(eng_grade)+'\n')
            self.read(delim='\n', expect='\n')

            self.read(length=57, expect='Enter the number of engineering hours for this resource: ')
            self.write('{}'.format(eng_hours)+ '\n')

            self.resource_entry_count+=1

            self.resource_entries.append((self.pop_start, self.pop_end, eng_grade, eng_hours))


        if self.resource_entry_count >= 600:
            self.read(length=35, expect='\nProposal resource limits reached\n\n')
        else:
            self.read(length=71, expect='\nEnter the estimate\'s start date (MM/DD/YYYY) or a blank line to exit: ')
            self.write('\n')


    def display_resource_data(self):

        self.read(delim=':', expect=':')
        self.write(random.choice('eE') + '\n')


        self.read(delim='\n', expect='\n')
        self.read(delim='\n', expect='Total number of resources: ' + '{}'.format(self.resource_entry_count) + '\n')

        i = 0
        while (i < self.resource_entry_count):
            temp_str='Entry: {}  Start: {}  End: {}  Grade: E0{}  Hours: {:,}'.format(i,
                    self.resource_entries[i][0].print_date(),
                    self.resource_entries[i][1].print_date(),
                    self.resource_entries[i][2],
                    self.resource_entries[i][3] )

            self.read(delim='\n', expect=temp_str + '\n')
            i+=1

        self.read_main_menu_text()


    def price_proposal(self):


        self.read(delim=':', expect=':')
        self.write(random.choice('fF') + '\n')

        if self.resource_entry_count==0:
            self.read(delim='\n', expect='\n')
            self.read(delim='\n', expect='No proposal resources have been entered\n')
            self.read(delim='\n', expect='\n')

            self.read_main_menu_text()

            return



        self.laborgraderates=[6550, 10010, 10522, 11069, 13599]

        self.fringe=3125;
        self.oh=6512;
        self.gna=811;

        total_direct_cost=0
        total_fringe=0
        total_oh=0
        total_gna=0
        total_burdened_labor=0
        total_profit = 0
        total_price = 0

        output_price = 0.0



        i = 0
        while (i < self.resource_entry_count):
            temp_str='Entry: {}  Start: {}  End: {}  Grade: E0{}  Hours: {:,}'.format(i,
                    self.resource_entries[i][0].print_date(),
                    self.resource_entries[i][1].print_date(),
                    self.resource_entries[i][2],
                    self.resource_entries[i][3] )

            laborgrade = self.resource_entries[i][2]

            direct_cost =  self.laborgraderates[laborgrade-1] * self.resource_entries[i][3]

            total_direct_cost+=direct_cost

            fringe = direct_cost * self.fringe / 10000

            total_fringe+= fringe

            oh_cost = (direct_cost + fringe) * self.oh / 10000

            total_oh += oh_cost

            gna_cost = (direct_cost + fringe + oh_cost) * self.gna / 10000

            total_gna += gna_cost

            burdened_labor = (direct_cost + fringe + oh_cost + gna_cost)

            total_burdened_labor += burdened_labor

            profit = burdened_labor * self.profit / 10000

            total_profit += profit

            price = burdened_labor + profit

            total_price += price

            i+=1

            # self.read(delim='\n', expect='Direct Labor: \t\t$ ' + '{:,.2f}'.format((direct_cost /100.0)) + '\n')
            # self.read(delim='\n', expect='Fringe: \t\t$ ' + '{:,.2f}'.format((fringe/100.0)) + '\n')
            # self.read(delim='\n', expect='Overhead: \t\t$ ' + '{:,.2f}'.format((oh_cost/ 100.0)) + '\n')
            # self.read(delim='\n', expect='G&A: \t\t\t$ ' + '{:,.2f}'.format((gna_cost / 100.0)) + '\n')
            # self.read(delim='\n', expect='\t\t\t-------------------\n')
            # self.read(delim='\n', expect='Burdened Labor: \t$ ' + '{:,.2f}'.format((burdened_labor/100.0)) + '\n')
            # self.read(delim='\n', expect='\n')
            # self.read(delim='\n', expect='Profit: \t\t$ ' + '{:,.2f}'.format((profit/100.0)) + '\n')
            # self.read(delim='\n', expect='\t\t\t-------------------\n');
            # self.read(delim='\n', expect='Price: \t\t\t$ '+ '{:,.2f}'.format((price/100.0)) + '\n')
            # self.read(delim='\n', expect='\n')




        self.read(delim='\n', expect='Total Direct Labor: \t$ ' + '{:,.2f}'.format((total_direct_cost /100.0)) + '\n')
        self.read(delim='\n', expect='Total Fringe: \t\t$ ' + '{:,.2f}'.format((total_fringe/100.0)) + '\n')
        self.read(delim='\n', expect='Total Overhead: \t$ ' + '{:,.2f}'.format((total_oh/ 100.0)) + '\n')
        self.read(delim='\n', expect='Total G&A: \t\t$ ' + '{:,.2f}'.format((total_gna / 100.0)) + '\n')
        self.read(delim='\n', expect= '\t\t\t-------------------\n')
        self.read(delim='\n', expect='Burdened Labor: \t$ ' + '{:,.2f}'.format((total_burdened_labor/100.0)) + '\n')
        self.read(delim='\n', expect='\n')
        self.read(delim='\n', expect='Total Profit: \t\t$ ' + '{:,.2f}'.format((total_profit/100.0)) + '\n')
        self.read(delim='\n', expect='\t\t\t-------------------\n')
        self.read(delim='\n', expect='Total Price: \t\t$ ' + '{:,.2f}'.format((total_price/100.0)) + '\n')
        self.read(delim='\n', expect='\n')

        self.read_main_menu_text()

class Dates():

    def __init__ (self, year=0, month=0, day=0):
        self.year = year
        self.month = month
        self.day = day
        self.day_of_year()

    def set_date(self, year, month, day):
        self.year=year
        self.month=month
        self.day=day


    def print_date(self):

        if self.year > 0:
            return '{}/{}/{}'.format(self.month, self.day, self.year)
        else:
            return ''

    def is_leap_year(self):
        if ((self.year%400==0 or self.year%100!=0) and (self.year%4==0)):
            return 1
        else:
            return 0

    def day_of_year(self):

        cumulative_days= [ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 ]
        self.DOY=cumulative_days[self.month-1] + self.day;

        if self.month > 2:
            self.DOY+=self.is_leap_year()

        return self.DOY

    def date_from_doy(self, year, doy):
        pass


    def remaining_days(self):


        if (self.is_leap_year()):
            return (366-self.DOY);
        else:
            return (365-self.DOY)


    def compare_dates(x, y):

        if x.year > y.year:
            return 1
        elif x.year < y.year:
            return -1
        else:
            if x.month > y.month:
                return 1
            elif x.month < y.month:
                return -1
            else:
                if x.day > y.day:
                    return 1
                elif x.day < y.day:
                    return -1
                else:
                    return 0

    @classmethod
    def random_date(cls):

        tmpyear=random.randint(2015, 2019)
        tmpmonth=random.randint(1, 12)
        tmpday=random.randint(1, 28)    # cheat by picking an upper day of month that will be safe for Feb
        return cls(tmpyear, tmpmonth, tmpday)

    @classmethod
    def random_date_from_range(cls, start_year, end_year):

        tmpyear=random.randint(start_year, end_year)
        tmpmonth=random.randint(1, 12)
        tmpday=random.randint(1, 28)
        return cls(tmpyear, tmpmonth, tmpday)


