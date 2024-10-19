#include "OrderBook.h"    // Includes the header file for the OrderBook class
#include "CSVReader.h"    // Includes the header file for the CSVReader class
#include <map>            // Includes the map container for storing key-value pairs
#include <algorithm>      // Includes the algorithm library for sorting operations
#include <iostream>       // Includes iostream for input/output operations

/** Constructor, reading a csv data file */
OrderBook::OrderBook(std::string filename) 
{
    // Initialize the orders vector by reading the CSV file using the CSVReader class
    orders = CSVReader::readCSV(filename);
}

/** Return vector of all known products in the dataset */
std::vector<std::string> OrderBook::getKnownProducts() 
{
    std::vector<std::string> products; // Stores the list of unique product names

    std::map<std::string, bool> prodMap; // A map to track the unique products

    // Loop through all orders and add product names to the map
    for (OrderBookEntry& e : orders) 
    {
        prodMap[e.product] = true; // Add product as key to the map, value is set to true
    }

    // Flatten the map (extract the keys) to a vector of strings
    for (auto const& e : prodMap) 
    {
        products.push_back(e.first); // Add the product name (key) to the products vector
    }

    return products; // Return the vector of unique products
}

/** Return vector of Orders according to the sent filters */
std::vector<OrderBookEntry> OrderBook::getOrders(OrderBookType type, 
                                                 std::string product, 
                                                 std::string timestamp) 
{
    std::vector<OrderBookEntry> orders_sub; // Stores the filtered list of orders

    // Loop through all orders and apply the filters
    for (OrderBookEntry& e : orders) 
    {
        // Check if the order matches the provided type, product, and timestamp
        if (e.orderType == type && 
            e.product == product && 
            e.timestamp == timestamp) 
        {
            orders_sub.push_back(e); // Add the matching order to the result list
        }
    }
    return orders_sub; // Return the filtered list of orders
}

/** Return the highest price in the given list of orders */
double OrderBook::getHighPrice(std::vector<OrderBookEntry>& orders) 
{
    double max = orders[0].price; // Initialize max with the price of the first order

    // Loop through the orders to find the highest price
    for (OrderBookEntry& e : orders) 
    {
        if (e.price > max) 
            max = e.price; // Update max if a higher price is found
    }
    return max; // Return the highest price
}

/** Return the lowest price in the given list of orders */
double OrderBook::getLowPrice(std::vector<OrderBookEntry>& orders) 
{
    double min = orders[0].price; // Initialize min with the price of the first order

    // Loop through the orders to find the lowest price
    for (OrderBookEntry& e : orders) 
    {
        if (e.price < min) 
            min = e.price; // Update min if a lower price is found
    }
    return min; // Return the lowest price
}

/** Return the earliest timestamp in the dataset */
std::string OrderBook::getEarliestTime() 
{
    return orders[0].timestamp; // Return the timestamp of the first order
}

/** Return the next timestamp after the given one */
std::string OrderBook::getNextTime(std::string timestamp) 
{
    std::string next_timestamp = ""; // Variable to store the next timestamp

    // Loop through the orders to find the next timestamp
    for (OrderBookEntry& e : orders) 
    {
        if (e.timestamp > timestamp) 
        {
            next_timestamp = e.timestamp; // Set next_timestamp to the first greater timestamp
            break; // Stop the loop once the next timestamp is found
        }
    }

    // If no later timestamp is found, return the earliest timestamp
    if (next_timestamp == "") 
    {
        next_timestamp = orders[0].timestamp;
    }
    return next_timestamp; // Return the next timestamp
}

/** Insert a new order into the order book */
void OrderBook::insertOrder(OrderBookEntry& order) 
{
    orders.push_back(order); // Add the new order to the list of orders

    // Sort the orders based on timestamp
    std::sort(orders.begin(), orders.end(), OrderBookEntry::compareByTimestamp);
}

/** Match asks to bids for a specific product at a specific timestamp */
std::vector<OrderBookEntry> OrderBook::matchAsksToBids(std::string product, std::string timestamp) 
{
    // Get all ask orders for the product at the timestamp
    std::vector<OrderBookEntry> asks = getOrders(OrderBookType::ask, product, timestamp);
    
    // Get all bid orders for the product at the timestamp
    std::vector<OrderBookEntry> bids = getOrders(OrderBookType::bid, product, timestamp);

    // Create a list to store the resulting sales
    std::vector<OrderBookEntry> sales; 

    // Check if there are bids and asks to process
    if (asks.size() == 0 || bids.size() == 0) 
    {
        std::cout << " OrderBook::matchAsksToBids no bids or asks" << std::endl;
        return sales; // Return empty sales if no bids or asks
    }

    // Sort asks in ascending order of price (lowest first)
    std::sort(asks.begin(), asks.end(), OrderBookEntry::compareByPriceAsc);

    // Sort bids in descending order of price (highest first)
    std::sort(bids.begin(), bids.end(), OrderBookEntry::compareByPriceDesc);

    // Log the highest/lowest ask and bid prices for debugging
    std::cout << "max ask " << asks[asks.size()-1].price << std::endl;
    std::cout << "min ask " << asks[0].price << std::endl;
    std::cout << "max bid " << bids[0].price << std::endl;
    std::cout << "min bid " << bids[bids.size()-1].price << std::endl;

    // Process matching of asks and bids
    for (OrderBookEntry& ask : asks) 
    {
        for (OrderBookEntry& bid : bids) 
        {
            // Check if the bid price is greater than or equal to the ask price (match found)
            if (bid.price >= ask.price) 
            {
                // Create a new sale with the matched price
                OrderBookEntry sale{ask.price, 0, timestamp, product, OrderBookType::asksale};

                // Handle specific user ("simuser") cases for assigning sale type
                if (bid.username == "simuser") 
                {
                    sale.username = "simuser";
                    sale.orderType = OrderBookType::bidsale;
                }
                if (ask.username == "simuser") 
                {
                    sale.username = "simuser";
                    sale.orderType = OrderBookType::asksale;
                }

                // Handle different matching cases based on bid and ask amounts
                if (bid.amount == ask.amount) 
                {
                    sale.amount = ask.amount; // Complete match
                    sales.push_back(sale);
                    bid.amount = 0; // Mark bid as processed
                    break; // Move to the next ask
                }

                if (bid.amount > ask.amount) 
                {
                    sale.amount = ask.amount; // Partial match
                    sales.push_back(sale);
                    bid.amount = bid.amount - ask.amount; // Adjust bid amount
                    break; // Move to the next ask
                }

                if (bid.amount < ask.amount && bid.amount > 0) 
                {
                    sale.amount = bid.amount; // Partial match
                    sales.push_back(sale);
                    ask.amount = ask.amount - bid.amount; // Adjust ask amount
                    bid.amount = 0; // Mark bid as processed
                    continue; // Continue with the next bid
                }
            }
        }
    }
    return sales; // Return the list of matched sales
}
