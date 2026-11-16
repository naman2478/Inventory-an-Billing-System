
#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <string>
#include <limits>
using namespace std;

// Money class
class Money {
private:
    double amount;

public:
    // Constructor
    Money(double a = 0) {
        amount = a;
    }

    double getAmount() const {
        return amount;
    }

    // Operator overloading
    Money operator+(const Money& m) const {
        return Money(amount + m.amount);
    }
};

// Product class
class Product {
private:
    int productID;
    string name;
    double price;
    int quantity;

public:
    // Constructor
    Product(int id = 0, string n = "",
            double p = 0, int q = 0) {
        productID = id;
        name = n;
        price = p;
        quantity = q;
    }

    int getID() const {
        return productID;
    }

    string getName() const {
        return name;
    }

    double getPrice() const {
        return price;
    }

    int getQuantity() const {
        return quantity;
    }

    // Update stock
    void updateStock(int q) {
        quantity -= q;
    }

    // Display product
    void display() const {
        cout << left << setw(10) << productID
             << setw(20) << name
             << setw(12) << fixed << setprecision(2)
             << price << quantity << endl;
    }
};

// Inventory class
class Inventory {
private:
    vector<Product> products;

public:

    // Add product
    void addProduct() {
        int id, qty;
        string name;
        double price;

        cout << "\nEnter Product ID: ";
        cin >> id;

        if (searchProduct(id) != NULL) {
            cout << "Product ID already exists!\n";
            return;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Enter Product Name: ";
        getline(cin, name);

        cout << "Enter Price: ";
        cin >> price;

        cout << "Enter Quantity: ";
        cin >> qty;

        if (id <= 0 || price < 0 || qty < 0 || name.empty()) {
            cout << "Invalid product details!\n";
            return;
        }

        products.push_back(Product(id, name, price, qty));

        cout << "Product added successfully!\n";
    }

    // Display all products
    void displayProducts() {
        if (products.empty()) {
            cout << "\nNo products available!\n";
            return;
        }

        cout << "\n========== PRODUCT LIST ==========\n";

        cout << left << setw(10) << "ID"
             << setw(20) << "Name"
             << setw(12) << "Price"
             << "Stock\n";

        for (vector<Product>::iterator it = products.begin();
             it != products.end(); it++) {

            it->display();
        }
    }

    // Search product by ID
    Product* searchProduct(int id) {
        for (vector<Product>::iterator it = products.begin();
             it != products.end(); it++) {

            if (it->getID() == id) {
                return &(*it);
            }
        }

        return NULL;
    }

    void searchByID() {
        int id;

        cout << "\nEnter Product ID to search: ";
        cin >> id;

        Product* p = searchProduct(id);

        if (p != NULL) {
            cout << "\nProduct Found!\n";

            cout << left << setw(10) << "ID"
                 << setw(20) << "Name"
                 << setw(12) << "Price"
                 << "Stock\n";

            p->display();
        }
        else {
            cout << "Product not found!\n";
        }
    }

    // Generate customer bill
    void generateBill() {
        int id, qty;
        Money subtotal(0);

        cout << "\n========== CUSTOMER BILL ==========\n";

        cout << left << setw(10) << "ID"
             << setw(20) << "Name"
             << setw(10) << "Qty"
             << "Amount\n";

        while (true) {
            cout << "\nEnter Product ID (0 to finish): ";
            cin >> id;

            if (id == 0) {
                break;
            }

            Product* p = searchProduct(id);

            if (p == NULL) {
                cout << "Product not found!\n";
                continue;
            }

            cout << "Enter Quantity: ";
            cin >> qty;

            if (qty <= 0) {
                cout << "Invalid quantity!\n";
                continue;
            }

            if (qty > p->getQuantity()) {
                cout << "Insufficient stock!\n";
                cout << "Available quantity: "
                     << p->getQuantity() << endl;
                continue;
            }

            double itemAmount = p->getPrice() * qty;

            cout << left << setw(10) << p->getID()
                 << setw(20) << p->getName()
                 << setw(10) << qty
                 << fixed << setprecision(2)
                 << itemAmount << endl;

            subtotal = subtotal + Money(itemAmount);

            // Update stock
            p->updateStock(qty);
        }

        cout << "\n-----------------------------------\n";

        cout << "Subtotal: Rs. "
             << fixed << setprecision(2)
             << subtotal.getAmount() << endl;

        cout << "Final Amount: Rs. "
             << subtotal.getAmount() << endl;

        cout << "-----------------------------------\n";

        saveToFile();

        cout << "Bill generated successfully!\n";
    }

    // Save products in file
    void saveToFile() {
        ofstream file("inventory.txt");

        if (!file) {
            cout << "Error saving inventory file!\n";
            return;
        }

        for (vector<Product>::iterator it = products.begin();
             it != products.end(); it++) {

            file << it->getID() << '\n';
            file << it->getName() << '\n';
            file << it->getPrice() << '\n';
            file << it->getQuantity() << '\n';
        }

        file.close();
    }

    // Load products from file
    void loadFromFile() {
        ifstream file("inventory.txt");

        if (!file) {
            return;
        }

        int id, qty;
        double price;
        string name;

        while (file >> id) {

            file.ignore(numeric_limits<streamsize>::max(), '\n');

            if (!getline(file, name)) {
                break;
            }

            if (!(file >> price >> qty)) {
                break;
            }

            file.ignore(numeric_limits<streamsize>::max(), '\n');

            if (id > 0 && price >= 0 && qty >= 0) {
                products.push_back(Product(id, name, price, qty));
            }
        }

        file.close();
    }
};

// Main function
int main() {

    Inventory inventory;

    inventory.loadFromFile();

    int choice;

    do {

        cout << "\n====================================\n";
        cout << "     INVENTORY AND BILLING SYSTEM\n";
        cout << "====================================\n";

        cout << "1. Add Product\n";
        cout << "2. Display Products\n";
        cout << "3. Search Product\n";
        cout << "4. Generate Customer Bill\n";
        cout << "5. Exit\n";

        cout << "\nEnter your choice: ";
        cin >> choice;

        // Using if-else instead of switch-case

        if (choice == 1) {

            inventory.addProduct();
            inventory.saveToFile();

        }

        else if (choice == 2) {

            inventory.displayProducts();

        }

        else if (choice == 3) {

            inventory.searchByID();

        }

        else if (choice == 4) {

            inventory.generateBill();

        }

        else if (choice == 5) {

            inventory.saveToFile();

            cout << "\nThank you for using "
                 << "Inventory and Billing System!\n";

        }

        else {

            cout << "\nInvalid choice! Please try again.\n";

        }

    } while (choice != 5);

    return 0;
}

