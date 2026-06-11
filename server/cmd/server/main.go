// cmd/server/main.go — Mock Server，Phase 1 完成

package main

import (
	"crypto/hmac"
	"crypto/sha256"
	"encoding/base64"
	"encoding/json"
	"log"
	"net/http"
	"os"
	"os/signal"
	"strings"
	"sync"
	"syscall"
	"time"

	"github.com/gorilla/websocket"
)

var jwtSecret = []byte("yunrong-mock-secret")

var upgrader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool { return true },
}

// ===== Hub =====

type Hub struct {
	mu      sync.RWMutex
	clients map[int64]*Client // userID → client
}

func newHub() *Hub  { return &Hub{clients: make(map[int64]*Client)} }

func (h *Hub) register(c *Client) {
	h.mu.Lock()
	defer h.mu.Unlock()
	if old, ok := h.clients[c.userID]; ok {
		old.conn.Close() // 踢掉旧连接
	}
	h.clients[c.userID] = c
	log.Printf("user %d registered (%d online)", c.userID, len(h.clients))
}

func (h *Hub) unregister(userID int64) {
	h.mu.Lock()
	defer h.mu.Unlock()
	delete(h.clients, userID)
	log.Printf("user %d unregistered (%d online)", userID, len(h.clients))
}

func (h *Hub) sendToUser(userID int64, msg []byte) {
	h.mu.RLock()
	defer h.mu.RUnlock()
	if c, ok := h.clients[userID]; ok {
		c.conn.WriteMessage(websocket.TextMessage, msg)
	}
}

type Client struct {
	conn   *websocket.Conn
	userID int64
}

// ===== main =====

func main() {
	hub := newHub()

	mux := http.NewServeMux()
	mux.HandleFunc("GET /health", healthHandler)
	mux.HandleFunc("POST /api/v1/auth/login", loginHandler)
	mux.HandleFunc("/ws", func(w http.ResponseWriter, r *http.Request) {
		wsHandler(w, r, hub)
	})

	addr := ":8080"
	log.Printf("Mock Server listening on %s", addr)

	go func() {
		if err := http.ListenAndServe(addr, mux); err != nil {
			log.Fatalf("server error: %v", err)
		}
	}()

	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit
	log.Println("Mock Server shutting down...")
}

// ===== handlers =====

func healthHandler(w http.ResponseWriter, r *http.Request) {
	writeJSON(w, http.StatusOK, map[string]string{"status": "ok"})
}

func loginHandler(w http.ResponseWriter, r *http.Request) {
	var req struct {
		Username string `json:"username"`
		Password string `json:"password"`
	}
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		writeJSON(w, http.StatusBadRequest, map[string]string{"error": "invalid json"})
		return
	}

	users := map[string]struct {
		password string
		userID   int64
		name     string
	}{
		"admin":    {"123456", 1001, "管理员"},
		"zhangsan": {"123456", 1002, "张三"},
		"lisi":     {"123456", 1003, "李四"},
	}

	u, ok := users[req.Username]
	if !ok || u.password != req.Password {
		writeJSON(w, http.StatusUnauthorized, map[string]string{"error": "invalid credentials"})
		return
	}

	token, err := makeJWT(u.userID, req.Username)
	if err != nil {
		writeJSON(w, http.StatusInternalServerError, map[string]string{"error": "token generation failed"})
		return
	}

	writeJSON(w, http.StatusOK, map[string]any{
		"code":    0,
		"message": "success",
		"data": map[string]any{
			"user_id":       u.userID,
			"display_name":  u.name,
			"access_token":  token,
			"refresh_token": token,
			"expires_in":    7200,
			"ws_url":        "ws://localhost:8080/ws",
		},
	})
}

func wsHandler(w http.ResponseWriter, r *http.Request, hub *Hub) {
	// 从 query 参数提取 token → userID
	token := r.URL.Query().Get("token")
	userID := parseTokenUserID(token)
	if userID == 0 {
		http.Error(w, "invalid or missing token", http.StatusUnauthorized)
		return
	}

	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Printf("ws upgrade failed: %v", err)
		return
	}

	client := &Client{conn: conn, userID: userID}
	hub.register(client)
	defer func() {
		hub.unregister(client.userID)
		conn.Close()
	}()

	log.Printf("ws connected: user %d from %s", userID, r.RemoteAddr)

	for {
		_, raw, err := conn.ReadMessage()
		if err != nil {
			log.Printf("ws disconnected: user %d (%v)", userID, err)
			return
		}

		var msg map[string]any
		if err := json.Unmarshal(raw, &msg); err != nil {
			log.Printf("ws non-JSON from user %d: %s", userID, string(raw))
			continue
		}

		log.Printf("ws recv user %d: %s", userID, string(raw))

		msgType, _ := msg["type"].(string)

		switch msgType {
		case "ping":
			conn.WriteJSON(map[string]any{"type": "pong"})

		case "msg":
			// 单聊转发：{type:"msg", to: <userID>, body: "..."}
			toUser := toFloat64(msg["to"])
			body, _ := msg["body"].(string)
			if toUser > 0 && body != "" {
				forward := map[string]any{
					"type": "msg",
					"from": userID,
					"body": body,
				}
				forwardBytes, _ := json.Marshal(forward)
				hub.sendToUser(int64(toUser), forwardBytes)
				log.Printf("msg routed: %d → %d", userID, int64(toUser))
			}

		default:
			conn.WriteJSON(map[string]any{"type": "echo", "payload": msg})
		}
	}
}

// ===== JWT =====

func makeJWT(userID int64, username string) (string, error) {
	header := base64.RawURLEncoding.EncodeToString(
		[]byte(`{"alg":"HS256","typ":"JWT"}`))
	payloadBytes, _ := json.Marshal(map[string]any{
		"user_id":  userID,
		"username": username,
		"exp":      time.Now().Add(2 * time.Hour).Unix(),
		"iat":      time.Now().Unix(),
	})
	payload := base64.RawURLEncoding.EncodeToString(payloadBytes)
	unsigned := header + "." + payload
	mac := hmac.New(sha256.New, jwtSecret)
	mac.Write([]byte(unsigned))
	sig := base64.RawURLEncoding.EncodeToString(mac.Sum(nil))
	return unsigned + "." + sig, nil
}

// parseTokenUserID 从 JWT 中提取 user_id（Mock 简化：仅解码 payload，不验签）
func parseTokenUserID(token string) int64 {
	parts := strings.Split(token, ".")
	if len(parts) != 3 {
		return 0
	}
	payload, err := base64.RawURLEncoding.DecodeString(parts[1])
	if err != nil {
		return 0
	}
	var claims map[string]any
	if err := json.Unmarshal(payload, &claims); err != nil {
		return 0
	}
	id, _ := claims["user_id"].(float64)
	return int64(id)
}

// toFloat64 converts any JSON number to float64
func toFloat64(v any) float64 {
	switch n := v.(type) {
	case float64:
		return n
	case float32:
		return float64(n)
	case json.Number:
		f, _ := n.Float64()
		return f
	}
	return 0
}

// ===== helpers =====

func writeJSON(w http.ResponseWriter, status int, v any) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(status)
	json.NewEncoder(w).Encode(v)
}

func init() {
	tok, _ := makeJWT(1001, "admin")
	tok2, _ := makeJWT(1002, "zhangsan")
	log.Println(strings.Repeat("=", 60))
	log.Println("Test tokens (valid 2h):")
	log.Printf("  admin:    ?token=%s", tok)
	log.Printf("  zhangsan: ?token=%s", tok2)
	log.Println(strings.Repeat("=", 60))
}
