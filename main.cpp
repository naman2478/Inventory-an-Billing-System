// ============================================================
//  Project 7 : Inventory and Billing System  (C++ / OOP)
//  Concepts  : Classes & objects, constructors, operator
//              overloading (Money), vectors, iterators,
//              file handling.
//  Menu uses a while loop + if / else-if (no switch).
//  Compile   : g++ -std=c++11 inventory_billing.cpp -o shop
// ============================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
using namespace std;

const string PRODUCT_FILE = "products.txt";
const string BILL_FILE    = "bills.txt";

// Optional, simple discount and tax settings
const long long DISCOUNT_LIMIT_RUPEES = 1000;  // discount if subtotal >= this
const int       DISCOUNT_PERCENT      = 5;
const int       TAX_PERCENT           = 5;

// ------------------------------------------------------------
//  Money : stores amount in paise to avoid floating point errors
// ------------------------------------------------------------
class Money {
    long long paise;
public:
    Money() : paise(0) {}                                  // default constructor
    explicit Money(long long p) : paise(p) {}              // from paise
    static Money fromRupees(double r) { return Money((long long)(r * 100 + 0.5)); }

    long long getPaise() const { return paise; }

    Money  operator+(const Money& o) const { return Money(paise + o.paise); }
    Money  operator-(const Money& o) const { return Money(paise - o.paise); }
    Money  operator*(int n) const          { return Money(paise * n); }
    Money& operator+=(const Money& o)      { paise += o.paise; return *this; }
    bool   operator>=(const Money& o) const { return paise >= o.paise; }
    bool   operator<(const Money& o) const  { return paise < o.paise; }

    Money percent(int pct) const { return Money(paise * pct / 100); }

    friend ostream& operator<<(ostream& os, const Money& m) {
        ostringstream s;
        s << "Rs. " << m.paise / 100 << "."
          << setw(2) << setfill('0') << m.paise % 100;
        return os << s.str();
    }
};

// ------------------------------------------------------------
//  Product : one item in the shop catalogue
// ------------------------------------------------------------
class Product {
    int    id;
    string name;
    Money  price;
    int    stock;
public:
    Product() : id(0), stock(0) {}
    Product(int i, const string& n, Money p, int s)
        : id(i), name(n), price(p), stock(s) {}

    int           getId()    const { return id; }
    const string& getName()  const { return name; }
    Money         getPrice() const { return price; }
    int           getStock() const { return stock; }

    void reduceStock(int q) { stock -= q; }
    void addStock(int q)    { stock += q; }

    void display() const {
        cout << left << setw(8) << id << setw(22) << name
             << right << setw(14);
        ostringstream p; p << price;
        cout << p.str() << setw(10) << stock << "\n";
    }

    // file format:  id|name|paise|stock
    void writeTo(ostream& os) const {
        os << id << '|' << name << '|' << price.getPaise() << '|' << stock << "\n";
    }
    bool readFrom(const string& line) {
        istringstream ss(line);
        string a, n, p, s;
        if (!getline(ss, a, '|') || !getline(ss, n, '|') ||
            !getline(ss, p, '|') || !getline(ss, s)) return false;
        id    = atoi(a.c_str());
        name  = n;
        price = Money(atoll(p.c_str()));
        stock = atoi(s.c_str());
        return true;
    }
};

// ------------------------------------------------------------
//  Inventory : manages vector<Product> and the product file
// ------------------------------------------------------------
class Inventory {
    vector<Product> items;
public:
    Inventory() { load(); }

    void load() {
        items.clear();
        ifstream in(PRODUCT_FILE.c_str());
        string line;
        while (getline(in, line)) {
            Product p;
            if (!line.empty() && p.readFrom(line)) items.push_back(p);
        }
    }

    void save() const {
        ofstream out(PRODUCT_FILE.c_str());
        for (vector<Product>::const_iterator it = items.begin(); it != items.end(); ++it)
            it->writeTo(out);
    }

    // returns pointer to product or NULL (uses iterator)
    Product* find(int id) {
        for (vector<Product>::iterator it = items.begin(); it != items.end(); ++it)
            if (it->getId() == id) return &(*it);
        return NULL;
    }

    bool add(const Product& p) {
        if (find(p.getId()) != NULL) return false;   // duplicate ID
        items.push_back(p);
        save();
        return true;
    }

    void displayAll() const {
        if (items.empty()) { cout << "No products in stock.\n"; return; }
        printHeader();
        for (vector<Product>::const_iterator it = items.begin(); it != items.end(); ++it)
            it->display();
    }

    static void printHeader() {
        cout << "\n" << left << setw(8) << "ID" << setw(22) << "Name"
             << right << setw(14) << "Price" << setw(10) << "Stock" << "\n";
        cout << string(54, '-') << "\n";
    }
};

// ------------------------------------------------------------
//  BillItem and Bill
// ------------------------------------------------------------
struct BillItem {
    int    id;
    string name;
    Money  price;
    int    qty;
    BillItem(int i, const string& n, Money p, int q) : id(i), name(n), price(p), qty(q) {}
    Money lineTotal() const { return price * qty; }
};

class Bill {
    vector<BillItem> lines;
    string customer;
public:
    explicit Bill(const string& c) : customer(c) {}

    bool empty() const { return lines.empty(); }

    int qtyAlreadyAdded(int id) const {
        for (vector<BillItem>::const_iterator it = lines.begin(); it != lines.end(); ++it)
            if (it->id == id) return it->qty;
        return 0;
    }

    void addItem(const Product& p, int qty) {
        for (vector<BillItem>::iterator it = lines.begin(); it != lines.end(); ++it)
            if (it->id == p.getId()) { it->qty += qty; return; }   // merge same product
        lines.push_back(BillItem(p.getId(), p.getName(), p.getPrice(), qty));
    }

    Money subtotal() const {
        Money total;
        for (vector<BillItem>::const_iterator it = lines.begin(); it != lines.end(); ++it)
            total += it->lineTotal();
        return total;
    }
    Money discount() const {
        Money sub = subtotal();
        if (sub >= Money(DISCOUNT_LIMIT_RUPEES * 100)) return sub.percent(DISCOUNT_PERCENT);
        return Money(0);
    }
    Money tax() const      { return (subtotal() - discount()).percent(TAX_PERCENT); }
    Money finalAmount() const { return subtotal() - discount() + tax(); }

    void print(ostream& os) const {
        os << "\n==================== BILL ====================\n";
        os << "Customer : " << customer << "\n";
        os << left << setw(6) << "ID" << setw(20) << "Item" << right
           << setw(5) << "Qty" << setw(13) << "Price" << setw(14) << "Amount" << "\n";
        os << string(58, '-') << "\n";
        for (vector<BillItem>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
            ostringstream p, a;
            p << it->price; a << it->lineTotal();
            os << left << setw(6) << it->id << setw(20) << it->name << right
               << setw(5) << it->qty << setw(13) << p.str() << setw(14) << a.str() << "\n";
        }
        os << string(58, '-') << "\n";
        printRow(os, "Subtotal", subtotal());
        printRow(os, "Discount (" + toStr(DISCOUNT_PERCENT) + "%)", discount());
        printRow(os, "Tax (" + toStr(TAX_PERCENT) + "%)", tax());
        printRow(os, "FINAL AMOUNT", finalAmount());
        os << "==============================================\n";
    }

    void appendToFile() const {
        ofstream out(BILL_FILE.c_str(), ios::app);
        print(out);
    }

    // reduce stock in inventory for every line
    void updateStock(Inventory& inv) const {
        for (vector<BillItem>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
            Product* p = inv.find(it->id);
            if (p) p->reduceStock(it->qty);
        }
        inv.save();
    }

private:
    static string toStr(int n) { ostringstream s; s << n; return s.str(); }
    static void printRow(ostream& os, const string& label, const Money& m) {
        ostringstream a; a << m;
        os << left << setw(40) << label << right << setw(18) << a.str() << "\n";
    }
};

// ------------------------------------------------------------
//  Input helpers
// ------------------------------------------------------------
int readInt(const string& prompt) {
    string line;
    while (true) {
        cout << prompt;
        getline(cin, line);
        istringstream ss(line);
        int v; char extra;
        if (ss >> v && !(ss >> extra)) return v;
        cout << "  Invalid number, try again.\n";
    }
}

double readDouble(const string& prompt) {
    string line;
    while (true) {
        cout << prompt;
        getline(cin, line);
        istringstream ss(line);
        double v; char extra;
        if (ss >> v && !(ss >> extra)) return v;
        cout << "  Invalid number, try again.\n";
    }
}

string readLine(const string& prompt) {
    string s;
    do {
        cout << prompt;
        getline(cin, s);
    } while (s.empty());
    return s;
}

// ------------------------------------------------------------
//  Menu actions
// ------------------------------------------------------------
void addProduct(Inventory& inv) {
    int id = readInt("Product ID      : ");
    if (id <= 0) { cout << "ID must be positive.\n"; return; }
    string name = readLine("Product name    : ");
    for (size_t i = 0; i < name.size(); ++i) if (name[i] == '|') name[i] = '-';
    double price = readDouble("Price (Rs.)     : ");
    int qty = readInt("Quantity in stock: ");
    if (price < 0 || qty < 0) { cout << "Price and quantity cannot be negative.\n"; return; }

    if (inv.add(Product(id, name, Money::fromRupees(price), qty)))
        cout << "Product added.\n";
    else
        cout << "A product with ID " << id << " already exists.\n";
}

void searchProduct(Inventory& inv) {
    int id = readInt("Enter product ID to search: ");
    Product* p = inv.find(id);
    if (p) { Inventory::printHeader(); p->display(); }
    else   cout << "Product not found.\n";
}

void makeBill(Inventory& inv) {
    Bill bill(readLine("Customer name : "));
    cout << "Enter product ID and quantity (ID 0 to finish).\n";

    while (true) {
        int id = readInt("Product ID (0 = done): ");
        if (id == 0) break;

        Product* p = inv.find(id);
        if (p == NULL) { cout << "  Product not found.\n"; continue; }

        int qty = readInt("Quantity for " + p->getName() + ": ");
        if (qty <= 0) { cout << "  Quantity must be positive.\n"; continue; }

        int available = p->getStock() - bill.qtyAlreadyAdded(id);
        if (qty > available) {
            cout << "  Not enough stock. Available: " << available << "\n";
            continue;
        }
        bill.addItem(*p, qty);
        cout << "  Added.\n";
    }

    if (bill.empty()) { cout << "No items on the bill.\n"; return; }

    bill.print(cout);
    bill.updateStock(inv);
    bill.appendToFile();
    cout << "Stock updated and bill saved to " << BILL_FILE << ".\n";
}

// ------------------------------------------------------------
//  main : loop-based menu using if / else-if
// ------------------------------------------------------------
int main() {
    Inventory inv;
    bool running = true;

    while (running) {
        cout << "\n====== SHOP INVENTORY & BILLING ======\n"
             << "1. Add product\n"
             << "2. Display all products\n"
             << "3. Search product by ID\n"
             << "4. Create customer bill\n"
             << "5. Exit\n";
        int choice = readInt("Enter choice: ");

        if (choice == 1)      addProduct(inv);
        else if (choice == 2) inv.displayAll();
        else if (choice == 3) searchProduct(inv);
        else if (choice == 4) makeBill(inv);
        else if (choice == 5) { inv.save(); cout << "Goodbye!\n"; running = false; }
        else                  cout << "Invalid choice, try again.\n";
    }
    return 0;
}
