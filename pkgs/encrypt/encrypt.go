package encrypt

import (
	"celer/pkgs/color"
	"os"
	"path/filepath"

	"golang.org/x/crypto/bcrypt"
)

func EncodePassword(password string) ([]byte, error) {
	hashed, err := bcrypt.GenerateFromPassword([]byte(password), bcrypt.DefaultCost)
	if err != nil {
		return nil, err
	}
	return hashed, nil
}

func CheckPassword(cacheDir, password string) bool {
	bytes, err := os.ReadFile(filepath.Join(cacheDir, "token"))
	if err != nil {
		color.Printf(color.Yellow, "failed to read cache token.\n %s", err)
		return false
	}
	return bcrypt.CompareHashAndPassword(bytes, []byte(password)) == nil
}
