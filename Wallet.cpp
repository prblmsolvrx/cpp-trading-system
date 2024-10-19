#include "Wallet.h"  // Include the Wallet class definition
#include <iostream>   // Include the input-output stream library for console operations
#include "CSVReader.h" // Include the CSVReader for reading and tokenizing strings

// Constructor for the Wallet class
Wallet::Wallet()
{
    // Empty constructor - no special initialization required here
}

// Method to insert currency into the wallet
void Wallet::insertCurrency(std::string type, double amount)
{
    double balance;  // Variable to store the balance for the currency type
    if (amount < 0)  // Check if the amount to be inserted is negative
    {
        throw std::exception{};  // Throw an exception for invalid (negative) amounts
    }
    if (currencies.count(type) == 0) // Check if the currency type does not exist in the wallet
    {
        balance = 0;  // Initialize balance to 0 if the currency type is not in the wallet
    }
    else {  // If the currency type already exists
        balance = currencies[type];  // Retrieve the current balance for the currency
    }
    balance += amount;  // Add the specified amount to the balance
    currencies[type] = balance;  // Update the balance for the currency in the wallet
}

// Method to remove currency from the wallet
bool Wallet::removeCurrency(std::string type, double amount)
{
    if (amount < 0)  // Check if the amount to be removed is negative
    {
        return false;  // Invalid amount, return false
    }
    if (currencies.count(type) == 0) // Check if the currency type does not exist in the wallet
    {
        return false;  // Currency not found, return false
    }
    else {  // If the currency type exists
        if (containsCurrency(type, amount)) // Check if the wallet contains enough currency
        {
            currencies[type] -= amount;  // Deduct the amount from the balance
            return true;  // Return true indicating successful removal
        }
        else  // If there isn't enough currency
            return false;  // Return false indicating insufficient balance
    }
}

// Method to check if the wallet contains a certain amount of currency
bool Wallet::containsCurrency(std::string type, double amount)
{
    if (currencies.count(type) == 0)  // Check if the currency type does not exist in the wallet
        return false;  // Currency not found, return false
    else 
        return currencies[type] >= amount;  // Return true if the wallet has enough currency
}

// Method to convert wallet information into a string representation
std::string Wallet::toString()
{
    std::string s;  // String to accumulate the wallet's currency information
    for (std::pair<std::string,double> pair : currencies)  // Iterate through all currencies in the wallet
    {
        std::string currency = pair.first;  // Get the currency type (key)
        double amount = pair.second;  // Get the amount of the currency (value)
        s += currency + " : " + std::to_string(amount) + "\n";  // Append the currency and its amount to the string
    }
    return s;  // Return the resulting string
}

// Method to check if the wallet can fulfill an order
bool Wallet::canFulfillOrder(OrderBookEntry order)
{
    std::vector<std::string> currs = CSVReader::tokenise(order.product, '/');  // Tokenize the product string to get currency types

    // If the order is an "ask" type
    if (order.orderType == OrderBookType::ask)
    {
        double amount = order.amount;  // Get the amount of currency to fulfill the order
        std::string currency = currs[0];  // The first currency in the pair
        std::cout << "Wallet::canFulfillOrder " << currency << " : " << amount << std::endl;

        return containsCurrency(currency, amount);  // Check if the wallet contains enough of the currency
    }
    // If the order is a "bid" type
    if (order.orderType == OrderBookType::bid)
    {
        double amount = order.amount * order.price;  // Calculate the total cost (amount * price)
        std::string currency = currs[1];  // The second currency in the pair
        std::cout << "Wallet::canFulfillOrder " << currency << " : " << amount << std::endl;
        return containsCurrency(currency, amount);  // Check if the wallet has enough funds to fulfill the order
    }

    return false;  // Default case, cannot fulfill the order
}

// Method to process a sale and update the wallet balances accordingly
void Wallet::processSale(OrderBookEntry& sale)
{
    std::vector<std::string> currs = CSVReader::tokenise(sale.product, '/');  // Tokenize the product string

    // If the sale is an "ask" type
    if (sale.orderType == OrderBookType::asksale)
    {
        double outgoingAmount = sale.amount;  // Amount to be removed from the wallet
        std::string outgoingCurrency = currs[0];  // The outgoing currency type
        double incomingAmount = sale.amount * sale.price;  // Amount to be added to the wallet
        std::string incomingCurrency = currs[1];  // The incoming currency type

        currencies[incomingCurrency] += incomingAmount;  // Add the incoming amount to the wallet
        currencies[outgoingCurrency] -= outgoingAmount;  // Subtract the outgoing amount from the wallet
    }
    // If the sale is a "bid" type
    if (sale.orderType == OrderBookType::bidsale)
    {
        double incomingAmount = sale.amount;  // Amount to be added to the wallet
        std::string incomingCurrency = currs[0];  // The incoming currency type
        double outgoingAmount = sale.amount * sale.price;  // Amount to be removed from the wallet
        std::string outgoingCurrency = currs[1];  // The outgoing currency type

        currencies[incomingCurrency] += incomingAmount;  // Add the incoming amount to the wallet
        currencies[outgoingCurrency] -= outgoingAmount;  // Subtract the outgoing amount from the wallet
    }
}

// Overloaded << operator to print wallet information using std::ostream
std::ostream& operator<<(std::ostream& os,  Wallet& wallet)
{
    os << wallet.toString();  // Output the string representation of the wallet
    return os;  // Return the ostream object
}
