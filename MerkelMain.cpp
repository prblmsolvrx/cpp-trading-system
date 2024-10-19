// Include header file for MerkelMain class
#include "MerkelMain.h"

// Include standard input/output library
#include <iostream>

// Include vector library for handling dynamic arrays
#include <vector>

// Include OrderBookEntry and CSVReader headers
#include "OrderBookEntry.h"
#include "CSVReader.h"

// Constructor for MerkelMain class
MerkelMain::MerkelMain()
{

}

// Function to initialize the program, handle input, and main event loop
void MerkelMain::init()
{
    int input; // Variable to store user input
    currentTime = orderBook.getEarliestTime(); // Get the earliest available time from the order book

    wallet.insertCurrency("BTC", 10); // Add 10 BTC to the wallet

    while(true) // Infinite loop to continuously display menu and process input
    {
        printMenu(); // Display the menu options
        input = getUserOption(); // Get user's selected option
        processUserOption(input); // Process the user's option
    }
}

// Function to print the main menu options
void MerkelMain::printMenu()
{
    // Print help option
    std::cout << "1: Print help " << std::endl;
    // Print exchange stats option
    std::cout << "2: Print exchange stats" << std::endl;
    // Print make an offer option
    std::cout << "3: Make an offer " << std::endl;
    // Print make a bid option
    std::cout << "4: Make a bid " << std::endl;
    // Print wallet details option
    std::cout << "5: Print wallet " << std::endl;
    // Continue to next timeframe
    std::cout << "6: Continue " << std::endl;

    // Print separator for clarity
    std::cout << "============== " << std::endl;

    // Display the current time from the order book
    std::cout << "Current time is: " << currentTime << std::endl;
}

// Function to print help message
void MerkelMain::printHelp()
{
    // Print a brief explanation of the game/objective
    std::cout << "Help - your aim is to make money. Analyse the market and make bids and offers. " << std::endl;
}

// Function to print the market statistics
void MerkelMain::printMarketStats()
{
    // Iterate over all known products in the order book
    for (std::string const& p : orderBook.getKnownProducts())
    {
        // Print the product name
        std::cout << "Product: " << p << std::endl;
        
        // Get all the ask orders for the product at the current time
        std::vector<OrderBookEntry> entries = orderBook.getOrders(OrderBookType::ask, p, currentTime);
        
        // Print the number of asks and the maximum and minimum ask prices
        std::cout << "Asks seen: " << entries.size() << std::endl;
        std::cout << "Max ask: " << OrderBook::getHighPrice(entries) << std::endl;
        std::cout << "Min ask: " << OrderBook::getLowPrice(entries) << std::endl;
    }
}

// Function to allow the user to make an ask (sell order)
void MerkelMain::enterAsk()
{
    // Prompt the user to enter product, price, and amount
    std::cout << "Make an ask - enter the amount: product,price, amount, eg  ETH/BTC,200,0.5" << std::endl;
    std::string input; // Variable to store user input
    std::getline(std::cin, input); // Get the input from the user

    // Tokenize the input string by commas
    std::vector<std::string> tokens = CSVReader::tokenise(input, ',');
    
    // Check if the input is valid (should have exactly 3 tokens)
    if (tokens.size() != 3)
    {
        // Print error message if input is invalid
        std::cout << "MerkelMain::enterAsk Bad input! " << input << std::endl;
    }
    else {
        try {
            // Convert the input strings into an OrderBookEntry
            OrderBookEntry obe = CSVReader::stringsToOBE(tokens[1], tokens[2], currentTime, tokens[0], OrderBookType::ask);
            obe.username = "simuser"; // Assign a username for the ask order

            // Check if the wallet has sufficient funds to fulfill the order
            if (wallet.canFulfillOrder(obe))
            {
                // If the wallet is valid, insert the order into the order book
                std::cout << "Wallet looks good. " << std::endl;
                orderBook.insertOrder(obe);
            }
            else {
                // Print message if wallet has insufficient funds
                std::cout << "Wallet has insufficient funds . " << std::endl;
            }
        }catch (const std::exception& e)
        {
            // Catch and handle any errors during the process
            std::cout << " MerkelMain::enterAsk Bad input " << std::endl;
        }   
    }
}

// Function to allow the user to make a bid (buy order)
void MerkelMain::enterBid()
{
    // Prompt the user to enter product, price, and amount
    std::cout << "Make an bid - enter the amount: product,price, amount, eg  ETH/BTC,200,0.5" << std::endl;
    std::string input; // Variable to store user input
    std::getline(std::cin, input); // Get the input from the user

    // Tokenize the input string by commas
    std::vector<std::string> tokens = CSVReader::tokenise(input, ',');
    
    // Check if the input is valid (should have exactly 3 tokens)
    if (tokens.size() != 3)
    {
        // Print error message if input is invalid
        std::cout << "MerkelMain::enterBid Bad input! " << input << std::endl;
    }
    else {
        try {
            // Convert the input strings into an OrderBookEntry
            OrderBookEntry obe = CSVReader::stringsToOBE(tokens[1], tokens[2], currentTime, tokens[0], OrderBookType::bid);
            obe.username = "simuser"; // Assign a username for the bid order

            // Check if the wallet has sufficient funds to fulfill the order
            if (wallet.canFulfillOrder(obe))
            {
                // If the wallet is valid, insert the order into the order book
                std::cout << "Wallet looks good. " << std::endl;
                orderBook.insertOrder(obe);
            }
            else {
                // Print message if wallet has insufficient funds
                std::cout << "Wallet has insufficient funds . " << std::endl;
            }
        }catch (const std::exception& e)
        {
            // Catch and handle any errors during the process
            std::cout << " MerkelMain::enterBid Bad input " << std::endl;
        }   
    }
}

// Function to print the wallet's current state
void MerkelMain::printWallet()
{
    // Print the contents of the wallet
    std::cout << wallet.toString() << std::endl;
}
        
// Function to move to the next timeframe and match orders
void MerkelMain::gotoNextTimeframe()
{
    // Notify the user that we are moving to the next timeframe
    std::cout << "Going to next time frame. " << std::endl;

    // Iterate over all known products
    for (std::string p : orderBook.getKnownProducts())
    {
        std::cout << "matching " << p << std::endl;

        // Match asks to bids for the current product and timeframe
        std::vector<OrderBookEntry> sales = orderBook.matchAsksToBids(p, currentTime);
        std::cout << "Sales: " << sales.size() << std::endl;

        // Process each sale and update the wallet if necessary
        for (OrderBookEntry& sale : sales)
        {
            std::cout << "Sale price: " << sale.price << " amount " << sale.amount << std::endl; 
            
            // If the user is involved in the sale, update the wallet
            if (sale.username == "simuser")
            {
                wallet.processSale(sale);
            }
        }
    }

    // Move to the next available time in the order book
    currentTime = orderBook.getNextTime(currentTime);
}
 
// Function to get the user's option from the menu
int MerkelMain::getUserOption()
{
    int userOption = 0; // Variable to store user option
    std::string line; // Variable to store user input as a string
    std::cout << "Type in 1-6" << std::endl;
    std::getline(std::cin, line); // Get the user's input

    try {
        userOption = std::stoi(line); // Convert the input string to an integer
    }catch(const std::exception& e)
    {
        // Handle any exceptions from the conversion
    }

    // Print the option chosen by the user
    std::cout << "You chose: " << userOption << std::endl;
    return userOption; // Return the chosen option
}

// Function to process the user's selected option from the menu
void MerkelMain::processUserOption(int userOption)
{
    // Check if the input is invalid
    if (userOption == 0) // bad input
    {
        std::cout << "Invalid choice. Choose 1-6" << std::endl;
    }
    if (userOption == 1) // Option 1: Print help
    {
        printHelp();
    }
    if (userOption == 2) // Option 2: Print market statistics
    {
        printMarketStats();
    }
    if (userOption == 3) // Option 3: Make an ask (sell order)
    {
        enterAsk();
    }
    if (userOption == 4) // Option 4: Make a bid (buy order)
    {
        enterBid();
    }
    if (userOption == 5) // Option 5: Print wallet contents
    {
        printWallet();
    }
    if (userOption == 6) // Option 6: Move to the next timeframe
    {
        gotoNextTimeframe();
    }       
}
