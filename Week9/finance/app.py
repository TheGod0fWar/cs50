import os

from cs50 import SQL
from flask import Flask, flash, redirect, render_template, request, session
from flask_session import Session
from werkzeug.security import check_password_hash, generate_password_hash

from helpers import apology, login_required, lookup, usd
from datetime import datetime

# Configure application
app = Flask(__name__)

# Custom filter
app.jinja_env.filters["usd"] = usd

# Configure session to use filesystem (instead of signed cookies)
app.config["SESSION_PERMANENT"] = False
app.config["SESSION_TYPE"] = "filesystem"
Session(app)

# Configure CS50 Library to use SQLite database
db = SQL("sqlite:///finance.db")


@app.after_request
def after_request(response):
    """Ensure responses aren't cached"""
    response.headers["Cache-Control"] = "no-cache, no-store, must-revalidate"
    response.headers["Expires"] = 0
    response.headers["Pragma"] = "no-cache"
    return response


@app.route("/")
@login_required
def index():
    """Show portfolio of stocks"""
    # Get user's purchases
    stocks = db.execute(
        "SELECT stock, SUM(number) as total_shares FROM purchase WHERE person_id = ? GROUP BY stock",
        session["user_id"],
    )

    # Get user's cash
    cash = db.execute("SELECT cash FROM users WHERE id = ?", session["user_id"])[0]["cash"]

    # Look up current prices and calculate totals
    total_stocks_value = 0
    for stock in stocks:
        quote = lookup(stock["stock"])
        if quote:
            stock["price"] = quote["price"]
            stock["total"] = stock["total_shares"] * quote["price"]
            total_stocks_value += stock["total"]
        else:
            # Handle case where lookup fails
            stock["price"] = 0
            stock["total"] = 0

    grand_total = cash + total_stocks_value

    return render_template(
        "index.html", stocks=stocks, cash=cash, grand_total=grand_total
    )


@app.route("/buy", methods=["GET", "POST"])
@login_required
def buy():
    """Buy shares of stock"""
    if request.method == "POST":
        # Get form data
        symbol = request.form.get("symbol")
        shares = request.form.get("shares")

        # Validate symbol
        if not symbol:
            return apology("must provide symbol")

        # Validate shares
        if not shares:
            return apology("must provide number of shares")

        # Ensure shares is a positive integer
        try:
            shares = int(shares)
            if shares <= 0:
                return apology("shares must be a positive integer")
        except ValueError:
            return apology("shares must be a positive integer")

        # Look up stock
        stock = lookup(symbol)
        if not stock:
            return apology("invalid symbol")

        # Get user's cash
        rows = db.execute("SELECT cash FROM users WHERE id = ?", session["user_id"])
        cash = rows[0]["cash"]

        # Calculate total cost
        total = stock["price"] * shares

        # Check if user can afford
        if cash < total:
            return apology("can't afford")

        # Record purchase
        now = datetime.now()
        sql_timestamp = now.strftime("%Y-%m-%d %H:%M:%S")

        # Create transactions table if it doesn't exist
        try:
            db.execute("""
                CREATE TABLE IF NOT EXISTS transactions (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    person_id INTEGER NOT NULL,
                    stock TEXT NOT NULL,
                    number INTEGER NOT NULL,
                    price NUMERIC NOT NULL,
                    time TIMESTAMP NOT NULL,
                    type TEXT NOT NULL
                )
            """)
        except Exception:
            pass

        # Insert into purchase table (for holdings)
        db.execute(
            "INSERT INTO purchase (time, person_id, price, stock, number) VALUES (?, ?, ?, ?, ?)",
            sql_timestamp,
            session["user_id"],
            stock["price"],
            symbol.upper(),
            shares
        )

        # Record in transactions table (for history)
        db.execute(
            "INSERT INTO transactions (person_id, stock, number, price, time, type) VALUES (?, ?, ?, ?, ?, ?)",
            session["user_id"],
            symbol.upper(),
            shares,
            stock["price"],
            sql_timestamp,
            "BUY"
        )

        # Update user's cash
        db.execute(
            "UPDATE users SET cash = cash - ? WHERE id = ?",
            total,
            session["user_id"]
        )

        return redirect("/")

    return render_template("buy.html")


@app.route("/history")
@login_required
def history():
    """Show history of transactions"""
    # Get all transactions for the user
    transactions = db.execute(
        "SELECT time as transacted, stock as symbol, number as shares, price, type FROM transactions WHERE person_id = ?",
        session["user_id"]
    )

    # Sort by time (most recent first)
    transactions.sort(key=lambda x: x["transacted"], reverse=True)

    return render_template("history.html", transactions=transactions)


@app.route("/change-password", methods=["GET", "POST"])
@login_required
def change_password():
    """Change user password"""
    if request.method == "POST":
        # Get form data
        current_password = request.form.get("current_password")
        new_password = request.form.get("new_password")
        confirmation = request.form.get("confirmation")

        # Validate inputs
        if not current_password:
            return apology("must provide current password")

        if not new_password:
            return apology("must provide new password")

        if not confirmation:
            return apology("must confirm new password")

        if new_password != confirmation:
            return apology("new passwords do not match")

        # Get current password hash from database
        rows = db.execute("SELECT hash FROM users WHERE id = ?", session["user_id"])

        # Verify current password
        if not check_password_hash(rows[0]["hash"], current_password):
            return apology("current password is incorrect")

        # Update password
        hash = generate_password_hash(new_password)
        db.execute("UPDATE users SET hash = ? WHERE id = ?", hash, session["user_id"])

        flash("Password changed successfully!")
        return redirect("/")

    return render_template("change_password.html")


@app.route("/login", methods=["GET", "POST"])
def login():
    """Log user in"""

    # Forget any user_id
    session.clear()

    # User reached route via POST (as by submitting a form via POST)
    if request.method == "POST":
        # Ensure username was submitted
        if not request.form.get("username"):
            return apology("must provide username", 403)

        # Ensure password was submitted
        elif not request.form.get("password"):
            return apology("must provide password", 403)

        # Query database for username
        rows = db.execute(
            "SELECT * FROM users WHERE username = ?", request.form.get("username")
        )

        # Ensure username exists and password is correct
        if len(rows) != 1 or not check_password_hash(
            rows[0]["hash"], request.form.get("password")
        ):
            return apology("invalid username and/or password", 403)

        # Remember which user has logged in
        session["user_id"] = rows[0]["id"]

        # Redirect user to home page
        return redirect("/")

    # User reached route via GET (as by clicking a link or via redirect)
    else:
        return render_template("login.html")


@app.route("/logout")
def logout():
    """Log user out"""

    # Forget any user_id
    session.clear()

    # Redirect user to login form
    return redirect("/")


@app.route("/quote", methods=["GET", "POST"])
@login_required
def quote():
    """Get stock quote."""
    if request.method == "POST":
        symbol = request.form.get("symbol")

        if not symbol:
            return apology("must provide symbol")

        stock = lookup(symbol)

        if not stock:
            return apology("invalid symbol")

        return render_template("quoted.html", stock=stock)

    return render_template("quote.html")


@app.route("/register", methods=["GET", "POST"])
def register():
    """Register user"""
    if request.method == "POST":
        # Get form data
        username = request.form.get("username")
        password = request.form.get("password")
        confirmation = request.form.get("confirmation")

        # Validate username
        if not username:
            return apology("must provide username")

        # Validate password
        if not password:
            return apology("must provide password")

        # Validate confirmation
        if not confirmation:
            return apology("must confirm password")

        # Check passwords match
        if password != confirmation:
            return apology("passwords do not match")

        # Try to insert user into database
        try:
            hash = generate_password_hash(password)
            db.execute("INSERT INTO users (username, hash) VALUES (?, ?)", username, hash)
            return redirect("/login")
        except ValueError:
            return apology("username already exists")

    return render_template("register.html")


@app.route("/sell", methods=["GET", "POST"])
@login_required
def sell():
    """Sell shares of stock"""
    # Get user's stocks for dropdown
    stocks = db.execute(
        "SELECT stock, SUM(number) as total_shares FROM purchase WHERE person_id = ? GROUP BY stock",
        session["user_id"]
    )

    if request.method == "POST":
        symbol = request.form.get("symbol")
        shares = request.form.get("shares")

        # Validate symbol
        if not symbol:
            return apology("must select a stock")

        # Validate shares
        if not shares:
            return apology("must provide number of shares")

        # Ensure shares is a positive integer
        try:
            shares = int(shares)
            if shares <= 0:
                return apology("shares must be a positive integer")
        except ValueError:
            return apology("shares must be a positive integer")

        # Check if user owns this stock and has enough shares
        user_stocks = db.execute(
            "SELECT SUM(number) as total_shares FROM purchase WHERE person_id = ? AND stock = ?",
            session["user_id"],
            symbol.upper()
        )

        if not user_stocks or user_stocks[0]["total_shares"] is None:
            return apology("you don't own this stock")

        owned_shares = user_stocks[0]["total_shares"]

        if owned_shares < shares:
            return apology("you don't own that many shares")

        # Look up current price
        stock = lookup(symbol)
        if not stock:
            return apology("invalid symbol")

        # Calculate sale value
        sale_value = stock["price"] * shares

        # Update purchase table - reduce shares
        # First, find the oldest purchases to reduce from
        purchases = db.execute(
            "SELECT id, number FROM purchase WHERE person_id = ? AND stock = ? ORDER BY time ASC",
            session["user_id"],
            symbol.upper()
        )

        shares_to_sell = shares
        for purchase in purchases:
            if shares_to_sell <= 0:
                break
            if purchase["number"] <= shares_to_sell:
                # Delete this purchase record
                db.execute("DELETE FROM purchase WHERE id = ?", purchase["id"])
                shares_to_sell -= purchase["number"]
            else:
                # Reduce the number of shares in this purchase
                db.execute(
                    "UPDATE purchase SET number = number - ? WHERE id = ?",
                    shares_to_sell,
                    purchase["id"]
                )
                shares_to_sell = 0

        # Update user's cash
        db.execute(
            "UPDATE users SET cash = cash + ? WHERE id = ?",
            sale_value,
            session["user_id"]
        )

        # Record the sale in transactions table
        now = datetime.now()
        sql_timestamp = now.strftime("%Y-%m-%d %H:%M:%S")

        # Create transactions table if it doesn't exist
        try:
            db.execute("""
                CREATE TABLE IF NOT EXISTS transactions (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    person_id INTEGER NOT NULL,
                    stock TEXT NOT NULL,
                    number INTEGER NOT NULL,
                    price NUMERIC NOT NULL,
                    time TIMESTAMP NOT NULL,
                    type TEXT NOT NULL
                )
            """)
        except Exception:
            pass

        # Insert the sell transaction
        db.execute(
            "INSERT INTO transactions (person_id, stock, number, price, time, type) VALUES (?, ?, ?, ?, ?, ?)",
            session["user_id"],
            symbol.upper(),
            shares,
            stock["price"],
            sql_timestamp,
            "SELL"
        )

        return redirect("/")

    return render_template("sell.html", stocks=stocks)
